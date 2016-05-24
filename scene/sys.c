#include "stm8s.h"
#include "CONST.h"
#include "ctype.h"
#include "macro.h"
#include "global.h"	
#include "debug.h"

#include "stm8s_adc2.h"

#include "arch.h"
#include "l_oled.h"
#include "l_fingprint.h"
#include "driver.h"
#include "sys.h"
#include "f_finger.h"

//unsigned int uiVOL_buffer[CVOL_BUFFER_SIZE + 1];

int sysProcess(msg_t *pMsg)
{
	int iRet = TRUE;
	
    u16 u16Ret = 0xffff;
    u16	u16Tmp;
	u16 temp1, temp2;
	msg_t msg;
	
	u8 ucArr[24];
    //u8 u8Ret = 0xff;
	
	switch(pMsg->msgType)
	{
	case CACT_TOUT:			/** һ�ζ������ **/
		actProcess(&g_actionQueue);
		break;
		
	case CPMT_TOUT:			/** һ��prompt������� **/
		actProcess(&g_promptQueue);
		break;
		
	case CBLK_TOUT:			/** һ��blink������� **/
		actProcess(&g_blinkQueue);
		break;
		
	case CBLK_OVER:			/** ȫ��blink������� **/
		blinkInit();
		break;
	
	case CADC_TOUT:
		temp1 = ADC2_GetConversionValue();
		adcSample_In(&g_adcData, temp1);
		
		//OLED_ShowNum12x24(2, 5, temp1, 7, 24);	//????????????????????????????????????????????
		//if((g_flag & (1<<8)) == 0) {			/** ADC��ѹ����������� **/
		if(adcSample_Out(&g_adcData,  &temp2) == TRUE) {		/** ADC��ѹ�������, ȡ��ֵ **/
			adcSample_Stop(&g_adcData);
			temp1 = VOL_getRefValue();
			if(temp1 < temp2) {
				g_flag &= ~(1 << 9);		/** ��õ�ѹ�ڲο���ѹ֮�� **/
			} else {
				g_flag |= (1 << 9);			/** �͵籨�� **/
			}
		}
		break;
	
	/***************************************************************************
	 * mcu��ָ�ƴ�������ͨ�ż������Σ�
	 *  ͨ�Ų�--���ݲ�--�����
	 * mcu��ָ�ƴ�������ͨ�����ݷ���:
	 * 1.ͨ��ʧ��(�ŵ����ϻ���ţ�����û�յ���ȷ����)���ɼ������ԡ�
	 * 2.ͨ��û���⣬ȡ��ͼ������ʧ�ܡ�ԭ����ָ�Ʋ�������ͼ����ȷ, �ȶ�ʱ�䳬ʱ(��������ε�)�ȡ��ɼ������ԡ�
	 *    (�ӷ���ʽָ�ƴ�����ʹ�÷����У����Բ²���Ϊ����ͼ����ȷ������ʱ�������ȶ�)��
	 * 3.�ɼ�����ȷ��ָ��ͼ�񣬱ȶԺ�û�з���ƥ���ģ�塣ȷ��Ϊһ��ʧ�ܡ�
	 * 4.��ȷ���أ��ҵ�ƥ���ָ��ģ�塣
	 ***************************************************************************/
	case CMSG_COMRX:
		//u16Ret = FP_GetRespData(0, ucArr);
		u16Ret = FP_GetRespData(0, ucArr);
		MFPACK_FIFO_CLEAN(x);
		if(u16Ret != 0) {			/** �յ���Ч���� **/
			u16Ret = FP_RespChk(ucArr);		/** data valid or not **/
			if(u16Ret  == CMD_ACK_SUM_ERR) {
				/** ָ�Ʋ�������-ʧ�� **/
				msg.msgType = CMSG_FGCOMFAIL;	//ͨ��У�����������
				msgq_in_irq(&g_msgq, &msg);
			} else if(u16Ret == CMD_ACK_OK) {
				/*******************************************
				 * now, we ensure that the data is valid 
				 * 
				 * we check several type of response
				 *******************************************/
				u16Ret = FP_RespGetType(ucArr);
				switch(u16Ret)
				{
				/***************************************************************
				 * ָ��¼���5�ַ��ؽ����
				 * 1. 0x00 - ָ���ɹ�
				 * 2. 0x01 - ָ���ʧ��
				 * 3. 0x60 - ָ����Template������Ч
				 * 4. 0x14 - ָ���������Ѵ���Template
				 * 5. 0xFFF1/0xFFF2/0xFFF3/0xFFF4 - ¼��ȴ�/�뿪��ָ
				 * 6. 0x23 - Timeoutʱ����û�м�⵽ָ�Ƶ�����
				 * 7. 0x21 - ָ��ͼ�񲻺�
				 * 8. 0x30 - �Ǽǵ�Template������ʧ��
				 ***************************************************************/
				case FP_ENROLL:
					u16Ret = FP_RespGetResult(ucArr);
					if(u16Ret == (u16)CRESP_ERR_SUCCESS) {
						/** ע��ָ��-�������� **/
						u16Ret = FP_RespGetLen(ucArr);
						if(u16Ret == 4) {
							u16Tmp = FP_RespGetId(ucArr);
							if(((u16Tmp >> 8 ) & 0xff) == 0xff) {
								if(((u16Tmp & 0xff) >= 0xf1) && ((u16Tmp & 0xff) <= 0xf4)) {
									msg.msgType = CMSG_FGRGSTING;
									msgq_in_irq(&g_msgq, &msg);
								}
							} else { }
						} else if(u16Ret == 6) { 
                    		/**  g_u16AddUserId = CINVALIDID; **/ 
                    		g_u16AddUserId = FP_RespGetId(ucArr);
                    		msg.msgType = CMSG_FGRGST;
							msgq_in_irq(&g_msgq, &msg);
						}
					} else {
						/** ע��ָ��-ʧ�ܷ��� **/
						u16Ret = FP_RespGetId(ucArr);
						if(u16Ret == CRESP_ERR_BAD_QUALITY) {
							/** ע��ָ��-ͼ��������ʱ **/
							msg.msgType = CMSG_FGRGSTING;
				   		 	msgq_in_irq(&g_msgq, &msg);
							break;
						} else if((u16Ret == CRESP_ERR_BAD_TIMEOUT)
								|| (u16Ret == CRESP_ERR_TMPL_NOT_EMPTY)
								|| (u16Ret == CRESP_ERR_GENERALIZE)
								|| (u16Ret == CRESP_ERR_INVALID_TMPL_NO)) {
							/** ע��ָ��-��ʱ **/	
							msg.msgType = CMSG_FGOPFAIL;
				   		 	msgq_in_irq(&g_msgq, &msg);
						} else {
							/** do nothing **/
						}
					}
					break;
					
				/***************************************************************
				 * ָ��ƥ���5�ַ��أ�
				 * 1. 0x00 - ָ���ɹ�
				 * 2. 0x01 - ָ���ʧ��
				 * 3. 0x15 - �������ѵǼǵ�ģ��(��Ϊ����ʧ��)
				 * 4. 0x23 - TimeOutʱ����û�м�⵽ָ�Ƶ�����(��Ϊ����ʧ��)
				 * 5. 0x21 - ָ��ͼ�񲻺�(ƥ�������)
				 * 6. 0xFFF4 - �뿪��ָ(ƥ�������)
				 * 7. 0x12 - �ѽ���1:N�ȶԣ�����ͬTemplate������(��Ϊ����ʧ��)
				 ***************************************************************/
				case FP_MATCH:
					u16Ret = FP_RespGetResult(ucArr);
					if(u16Ret == (u16)CRESP_ERR_SUCCESS) {
						/** ����ָ��-�ɹ� **/
						u16Ret = FP_RespGetLen(ucArr);
						if(u16Ret == 4) {
							u16Tmp = FP_RespGetId(ucArr);
							if(u16Tmp  == 0xfff4) {
								msg.msgType = CMSG_FGQUERYING;		//��ѯ������
								msgq_in_irq(&g_msgq, &msg);
							} else {
								MFPid_PUSH(u16Tmp);
							
								msg.msgValue = u16Tmp;
								msg.msgType = CMSG_FGQUERY;			//�ɹ���ѯ��һ��ָ��ģ��id
								msgq_in_irq(&g_msgq, &msg);
							}
						}
					} else {
						/** ����ʧ�� **/
						u16Tmp = FP_RespGetId(ucArr);
						if(u16Tmp == CRESP_ERR_BAD_QUALITY) {
							/** ƥ��ָ��-ͼ���������� **/	
							msg.msgType = CMSG_FGQUERYING;	//�͹�ԭ����ɵ�ʧ�ܣ���������һ��
				   	 		msgq_in_irq(&g_msgq, &msg);
				   		} else if((u16Tmp == CRESP_ERR_BAD_TIMEOUT)
				   					|| (u16Tmp == CRESP_ERR_IDENTIFY)
				   					|| (u16Tmp == CRESP_ERR_ALL_TMPL_EMPTY)){
				   			/** ƥ��ָ��-�����ڻ�ʱ��ģ��� **/
							msg.msgType = CMSG_FGOPFAIL;	//��ѯʧ��
				   	 		msgq_in_irq(&g_msgq, &msg);
				   		} else {
				   			/** do nothing **/
				   		}
					}
					break;
					
				case FP_DELETE:		/** ��Ҫ��ѯ����״̬ȷ�ϲ����ɹ�����? **/
					/** ɾ��ָ��-�ɹ� **/
					u16Ret = FP_RespGetResult(ucArr);
					if(u16Ret == (u16)CRESP_ERR_SUCCESS) {
						/** ɾ��ָ��-�������� **/
						msg.msgType = CMSG_FGDERG;
						msgq_in_irq(&g_msgq, &msg);
					} else {
						/** ɾ��ָ��-ʧ�ܷ��� **/
						u16Ret = FP_RespGetId(ucArr);
						if((u16Ret == CRESP_ERR_INVALID_TMPL_NO)
								|| (u16Ret == CRESP_ERR_TMPL_EMPTY)) {
							/**  ɾ��ָ��-�����Ч��ģ���Ҳ��Ϊ�ɹ� **/
							msg.msgType = CMSG_FGDERG;
				   		 	msgq_in_irq(&g_msgq, &msg);
						} else {
							/** ����������Ϊʧ��**/
							msg.msgType = CMSG_FGOPFAIL;
				   		 	msgq_in_irq(&g_msgq, &msg);
						}
					}
					break;
					
				case FP_DELALL:		/** ��Ҫ��ѯ����״̬ȷ�ϲ����ɹ�����? **/
					u16Ret = FP_RespGetResult(ucArr);
					if(u16Ret == (u16)CRESP_ERR_SUCCESS) {
						/** ɾ��ָ��-�ɹ� **/
						msg.msgType = CMSG_FGDERG;
						msgq_in_irq(&g_msgq, &msg);
					} else {
						/** ɾ��ָ��-ʧ�� **/
						msg.msgType = CMSG_FGOPFAIL;
				   	 	msgq_in_irq(&g_msgq, &msg);
					}
					break;
					
				case FP_TMPSTATE:		/** get the state with specified template **/
					/** ָ��ģ���Ƿ���� **/
					u16Ret = FP_RespGetResult(ucArr);
					if(u16Ret == (u16)CRESP_ERR_SUCCESS) {
						/** ��ָ��ģ��״̬-�ɹ� **/
						u16Ret = FP_RespGetId(ucArr);
						if(u16Ret == CRESP_GD_TEMPLATE_NOT_EMPTY) {
							msg.msgType = CMSG_FGTMPBUSY;
							msgq_in_irq(&g_msgq, &msg);
						} else {
							msg.msgType = CMSG_FGTMPEMPTY;
							msgq_in_irq(&g_msgq, &msg);
						}
					} else {
						/** ��ָ��ģ��״̬-ʧ�� **/
						msg.msgType = CMSG_FGOPFAIL;
				   	 	msgq_in_irq(&g_msgq, &msg);
					}
					break;
					
				case FP_TMPINTEG:		/** get the integrity with specified template **/
					/** ָ��ģ�������� **/
					u16Ret = FP_RespGetId(ucArr);		//����ģ����
					u16Tmp = FP_RespGetIdAttach(ucArr);	//��һ������ģ����
					if((u16Ret > 0) && (u16Tmp > 0)) {
						msg.msgType = CMSG_FGNOTINTEGR;
						msgq_in_irq(&g_msgq, &msg);
					} else {
						msg.msgType = CMSG_FGINTEGRITY;
						msgq_in_irq(&g_msgq, &msg);
					}
					break;
				
				case FP_INCORRECT_COMMAND:
					/** incorrect command **/
					msg.msgType = CMSG_FGDOING;
					msgq_in_irq(&g_msgq, &msg);
					break;
					
				case FP_CANCEL:
					/**  **/
					msg.msgType = CMSG_FGINIT;
					//msg.msgType = CMSG_FGCCEL;
					msgq_in_irq(&g_msgq, &msg);
					
					break;
					
				default:
					break;
				}
			}
		}
		break;
		
	case CMSG_UART3RX:	
		u16Ret = BT_GetRespData(ucArr);
    	MBTACK_FIFO_CLEAN("����������ջ�����");
		if(u16Ret != 0) {						/** received data **/
			
			u16Ret = BT_RespGetLen(ucArr);
			u16Ret = (u16Ret <= 24)? u16Ret : 24;
			for(u16Tmp = 0; u16Tmp < u16Ret; u16Tmp++) {
				g_u8bluetooth[u16Tmp] = ucArr[u16Tmp + 2];
			}
			
			u16Ret = BT_RespChksum(ucArr);		/** data valid or not **/
			if(u16Ret  == CMD_ACK_SUM_ERR) {
				/** ��Ч���� **/
				//msg.msgType = CMSG_FGCOMFAIL;	//ͨ��У�����������
				//msgq_in_irq(&g_msgq, &msg);
				
				/** if ignore the chksum ***********************************************************/
				for(u16Ret = 0; ((u16Ret < BT_RespGetLen(ucArr)) && (u16Ret < (16 + 1))); u16Ret++) {
					g_u8bluetooth[u16Ret] = ucArr[u16Ret + 2];
				}
				
				u16Ret = BT_RespGetType(ucArr);
				if(u16Ret == CCMD_ENROLL) {
					//msg.msgType = CMSG_BTMATCH;		//received bluetooth match data
					msg.msgType = CMSG_BTENROLL;	//received bluetooth enroll data
					msgq_in_irq(&g_msgq, &msg);
				} else if(u16Ret == CCMD_MATCH) {
					msg.msgType = CMSG_BTMATCH;		//received bluetooth match data
					msgq_in_irq(&g_msgq, &msg);
				} else { /** do nothing **/ }
				/***********************************************************************************/
			} else {	
				/** ������Ч **/
				for(u16Ret = 0; ((u16Ret < BT_RespGetLen(ucArr)) && (u16Ret < (16 + 1))); u16Ret++) {
					g_u8bluetooth[u16Ret] = ucArr[u16Ret + 2];
				}
				
				u16Ret = BT_RespGetType(ucArr);
				if(u16Ret == CCMD_ENROLL) {
					msg.msgType = CMSG_BTENROLL;	//received bluetooth enroll data
					msgq_in_irq(&g_msgq, &msg);
				} else if(u16Ret == CCMD_MATCH) {
					msg.msgType = CMSG_BTMATCH;		//received bluetooth match data
					msgq_in_irq(&g_msgq, &msg);
				} else { /** do nothing **/ }
			}
		}
		break;
		
	default:
		/** Can't process the msg here **/
		iRet = FALSE;
		break;
	}
	
	return  iRet;
}

/*******************************************************************************/