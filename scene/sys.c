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
	case CACT_TOUT:			/** 一段动作完成 **/
		actProcess(&g_actionQueue);
		break;
		
	case CPMT_TOUT:			/** 一段prompt动作完成 **/
		actProcess(&g_promptQueue);
		break;
		
	case CBLK_TOUT:			/** 一段blink动作完成 **/
		actProcess(&g_blinkQueue);
		break;
		
	case CBLK_OVER:			/** 全部blink动作完成 **/
		blinkInit();
		break;
	
	case CADC_TOUT:
		temp1 = ADC2_GetConversionValue();
		adcSample_In(&g_adcData, temp1);
		
		//OLED_ShowNum12x24(2, 5, temp1, 7, 24);	//????????????????????????????????????????????
		//if((g_flag & (1<<8)) == 0) {			/** ADC电压采样完成了吗？ **/
		if(adcSample_Out(&g_adcData,  &temp2) == TRUE) {		/** ADC电压采样完成, 取均值 **/
			adcSample_Stop(&g_adcData);
			temp1 = VOL_getRefValue();
			if(temp1 < temp2) {
				g_flag &= ~(1 << 9);		/** 测得电压在参考电压之上 **/
			} else {
				g_flag |= (1 << 9);			/** 低电报警 **/
			}
		}
		break;
	
	/***************************************************************************
	 * mcu与指纹传感器的通信及处理层次：
	 *  通信层--数据层--处理层
	 * mcu与指纹传感器的通信数据分类:
	 * 1.通信失败(信道故障或干扰，导致没收到正确数据)。可继续偿试。
	 * 2.通信没问题，取得图像数据失败。原因，如指纹不清晰，图像不正确, 比对时间超时(传感器层次的)等。可继续偿试。
	 *    (从方程式指纹传感器使用反馈中，可以猜测认为，在图象不正确或不清晰时，持续比对)。
	 * 3.采集到正确的指纹图像，比对后没有发现匹配的模板。确认为一次失败。
	 * 4.正确返回，找到匹配的指纹模板。
	 ***************************************************************************/
	case CMSG_COMRX:
		//u16Ret = FP_GetRespData(0, ucArr);
		u16Ret = FP_GetRespData(0, ucArr);
		MFPACK_FIFO_CLEAN(x);
		if(u16Ret != 0) {			/** 收到有效数据 **/
			u16Ret = FP_RespChk(ucArr);		/** data valid or not **/
			if(u16Ret  == CMD_ACK_SUM_ERR) {
				/** 指纹操作数据-失败 **/
				msg.msgType = CMSG_FGCOMFAIL;	//通信校验错。可以重试
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
				 * 指纹录入的5种返回结果：
				 * 1. 0x00 - 指令处理成功
				 * 2. 0x01 - 指令处理失败
				 * 3. 0x60 - 指定的Template号码无效
				 * 4. 0x14 - 指定号码中已存在Template
				 * 5. 0xFFF1/0xFFF2/0xFFF3/0xFFF4 - 录入等待/离开手指
				 * 6. 0x23 - Timeout时间内没有检测到指纹的输入
				 * 7. 0x21 - 指纹图像不好
				 * 8. 0x30 - 登记的Template的制作失败
				 ***************************************************************/
				case FP_ENROLL:
					u16Ret = FP_RespGetResult(ucArr);
					if(u16Ret == (u16)CRESP_ERR_SUCCESS) {
						/** 注册指纹-正常返回 **/
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
						/** 注册指纹-失败返回 **/
						u16Ret = FP_RespGetId(ucArr);
						if(u16Ret == CRESP_ERR_BAD_QUALITY) {
							/** 注册指纹-图象不清晰或超时 **/
							msg.msgType = CMSG_FGRGSTING;
				   		 	msgq_in_irq(&g_msgq, &msg);
							break;
						} else if((u16Ret == CRESP_ERR_BAD_TIMEOUT)
								|| (u16Ret == CRESP_ERR_TMPL_NOT_EMPTY)
								|| (u16Ret == CRESP_ERR_GENERALIZE)
								|| (u16Ret == CRESP_ERR_INVALID_TMPL_NO)) {
							/** 注册指纹-超时 **/	
							msg.msgType = CMSG_FGOPFAIL;
				   		 	msgq_in_irq(&g_msgq, &msg);
						} else {
							/** do nothing **/
						}
					}
					break;
					
				/***************************************************************
				 * 指纹匹配的5种返回：
				 * 1. 0x00 - 指令处理成功
				 * 2. 0x01 - 指令处理失败
				 * 3. 0x15 - 不存在已登记的模板(视为处理失败)
				 * 4. 0x23 - TimeOut时间内没有检测到指纹的输入(视为处理失败)
				 * 5. 0x21 - 指纹图像不好(匹配进行中)
				 * 6. 0xFFF4 - 离开手指(匹配进行中)
				 * 7. 0x12 - 已进行1:N比对，但相同Template不存在(视为处理失败)
				 ***************************************************************/
				case FP_MATCH:
					u16Ret = FP_RespGetResult(ucArr);
					if(u16Ret == (u16)CRESP_ERR_SUCCESS) {
						/** 查找指纹-成功 **/
						u16Ret = FP_RespGetLen(ucArr);
						if(u16Ret == 4) {
							u16Tmp = FP_RespGetId(ucArr);
							if(u16Tmp  == 0xfff4) {
								msg.msgType = CMSG_FGQUERYING;		//查询进行中
								msgq_in_irq(&g_msgq, &msg);
							} else {
								MFPid_PUSH(u16Tmp);
							
								msg.msgValue = u16Tmp;
								msg.msgType = CMSG_FGQUERY;			//成功查询到一个指纹模板id
								msgq_in_irq(&g_msgq, &msg);
							}
						}
					} else {
						/** 查找失败 **/
						u16Tmp = FP_RespGetId(ucArr);
						if(u16Tmp == CRESP_ERR_BAD_QUALITY) {
							/** 匹配指纹-图像质量不好 **/	
							msg.msgType = CMSG_FGQUERYING;	//客观原因造成的失败，可以再来一次
				   	 		msgq_in_irq(&g_msgq, &msg);
				   		} else if((u16Tmp == CRESP_ERR_BAD_TIMEOUT)
				   					|| (u16Tmp == CRESP_ERR_IDENTIFY)
				   					|| (u16Tmp == CRESP_ERR_ALL_TMPL_EMPTY)){
				   			/** 匹配指纹-不存在或超时或模板空 **/
							msg.msgType = CMSG_FGOPFAIL;	//查询失败
				   	 		msgq_in_irq(&g_msgq, &msg);
				   		} else {
				   			/** do nothing **/
				   		}
					}
					break;
					
				case FP_DELETE:		/** 需要查询返回状态确认操作成功了吗? **/
					/** 删除指纹-成功 **/
					u16Ret = FP_RespGetResult(ucArr);
					if(u16Ret == (u16)CRESP_ERR_SUCCESS) {
						/** 删除指纹-正常返回 **/
						msg.msgType = CMSG_FGDERG;
						msgq_in_irq(&g_msgq, &msg);
					} else {
						/** 删除指纹-失败返回 **/
						u16Ret = FP_RespGetId(ucArr);
						if((u16Ret == CRESP_ERR_INVALID_TMPL_NO)
								|| (u16Ret == CRESP_ERR_TMPL_EMPTY)) {
							/**  删除指纹-编号无效或模板空也视为成功 **/
							msg.msgType = CMSG_FGDERG;
				   		 	msgq_in_irq(&g_msgq, &msg);
						} else {
							/** 其它错误视为失败**/
							msg.msgType = CMSG_FGOPFAIL;
				   		 	msgq_in_irq(&g_msgq, &msg);
						}
					}
					break;
					
				case FP_DELALL:		/** 需要查询返回状态确认操作成功了吗? **/
					u16Ret = FP_RespGetResult(ucArr);
					if(u16Ret == (u16)CRESP_ERR_SUCCESS) {
						/** 删除指纹-成功 **/
						msg.msgType = CMSG_FGDERG;
						msgq_in_irq(&g_msgq, &msg);
					} else {
						/** 删除指纹-失败 **/
						msg.msgType = CMSG_FGOPFAIL;
				   	 	msgq_in_irq(&g_msgq, &msg);
					}
					break;
					
				case FP_TMPSTATE:		/** get the state with specified template **/
					/** 指纹模板是否空闲 **/
					u16Ret = FP_RespGetResult(ucArr);
					if(u16Ret == (u16)CRESP_ERR_SUCCESS) {
						/** 查指纹模板状态-成功 **/
						u16Ret = FP_RespGetId(ucArr);
						if(u16Ret == CRESP_GD_TEMPLATE_NOT_EMPTY) {
							msg.msgType = CMSG_FGTMPBUSY;
							msgq_in_irq(&g_msgq, &msg);
						} else {
							msg.msgType = CMSG_FGTMPEMPTY;
							msgq_in_irq(&g_msgq, &msg);
						}
					} else {
						/** 查指纹模板状态-失败 **/
						msg.msgType = CMSG_FGOPFAIL;
				   	 	msgq_in_irq(&g_msgq, &msg);
					}
					break;
					
				case FP_TMPINTEG:		/** get the integrity with specified template **/
					/** 指纹模板完整性 **/
					u16Ret = FP_RespGetId(ucArr);		//已损坏模板数
					u16Tmp = FP_RespGetIdAttach(ucArr);	//第一个已损坏模板编号
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
    	MBTACK_FIFO_CLEAN("清空蓝牙接收缓冲区");
		if(u16Ret != 0) {						/** received data **/
			
			u16Ret = BT_RespGetLen(ucArr);
			u16Ret = (u16Ret <= 24)? u16Ret : 24;
			for(u16Tmp = 0; u16Tmp < u16Ret; u16Tmp++) {
				g_u8bluetooth[u16Tmp] = ucArr[u16Tmp + 2];
			}
			
			u16Ret = BT_RespChksum(ucArr);		/** data valid or not **/
			if(u16Ret  == CMD_ACK_SUM_ERR) {
				/** 无效数据 **/
				//msg.msgType = CMSG_FGCOMFAIL;	//通信校验错。可以重试
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
				/** 操作有效 **/
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