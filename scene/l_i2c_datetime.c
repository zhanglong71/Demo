
#include "stm8s.h"
#include "CONST.h"
#include "ctype.h"
#include "macro.h"
#include "global.h"	
#include "debug.h"

#include "arch.h"
#include "l_i2c_datetime.h"

/**
 * ��ʼ֮��scl/sda������L
 **/
void datetime_iic_start()
{
	MDATETIME_SDA_H("1. sda=1");
	//delaySpin(6);
	MDATETIME_SCL_H("2. scl=1");
	delaySpin(13);
	MDATETIME_SDA_L("3. sda=0");
	delaySpin(6);
	MDATETIME_SCL_L("4. scl=0");
}

/**
 * ����֮��scl/sda������H
 **/
void datetime_iic_stop()
{
	MDATETIME_SDA_L("1. sda = 0");
	nop();nop();
	MDATETIME_SCL_H("2. scl = 1");
	nop();nop();
	MDATETIME_SDA_H("3. sda = 1");
	nop();nop();
}

/***********************************************************
 * д1��byte(���һ��byte�ķ��ͺ�SCLΪL)
 ***********************************************************/
u8 datetime_iic_writebyte(u8 Data)
{
	u8 i;
	//u8 Ack_Bit;                  //Ӧ���ź�
	for(i = 0; i < 8; i++) {
		if(Data & 0x80)MDATETIME_SDA_H("�����");else MDATETIME_SDA_L("�����");
		MDATETIME_SCL_H("������Ч");
		delaySpin(10);
		MDATETIME_SCL_L("׼����һ��bit����");
		delaySpin(6);
		Data <<= 1;
	}
	
	return Data;	
}

/***********************************************************
 * ��1��byte����(����һ��byte��SCLΪL)
 ***********************************************************/
u8 datetime_iic_readbyte()
{
	u8 i;
	u8 Data = 0;
	
	MDATETIME_SDA_IN("sda��Ϊ����");
	MDATETIME_SCL_OUT("scl��Ϊ���");
	for(i = 0; i < 8; i++) {
		delaySpin(6);
		MDATETIME_SCL_H("׼����");
		delaySpin(6);
		Data <<= 1;
		if(MDATETIME_READ_SDA("��sda")){Data |= 0x01; } else { Data &= 0xfe; }
		MDATETIME_SCL_L("׼������һ��bit");
	}
	MDATETIME_SDA_OUT("sda��Ϊ���");
	
	return Data;	
}
/***********************************************************
 * ��Ӧ���ź�(��writeһ��byte֮��)
 ***********************************************************/
u8 datetime_iic_readACK()
{
    u8 Ack_Bit;          			//Ӧ���ź�
        
	MDATETIME_SDA_H("�ͷ�sda");		//�ͷ�IIC SDA����Ϊ���������մ���������Ӧ���ź�	
	delaySpin(5);
	MDATETIME_SCL_H("���������");         	//��9��ʱ������
	delaySpin(12);
	Ack_Bit = MDATETIME_READ_SDA("��sda");	//��ȡӦ���ź�
	MDATETIME_SCL_L("���");
        
    return  Ack_Bit;
}

/***********************************************************
 * дACKӦ���ź�(��readһ��byte֮��)
 ***********************************************************/
void datetime_iic_writeACK()
{
	MDATETIME_SDA_L("׼��ACK");
	delaySpin(5);
	MDATETIME_SCL_H("ACK��Ч");
	delaySpin(12);
	MDATETIME_SCL_L("���");
	delaySpin(5);
}
/***********************************************************
 * дNCKӦ���ź�(��read���һ��byte֮��)
 ***********************************************************/
void datetime_iic_writeNCK()
{
	MDATETIME_SDA_H("׼��NCK");
	nop();nop();
	MDATETIME_SCL_H("NCK��Ч");
	delaySpin(12);
	MDATETIME_SCL_L("���");
	delaySpin(5);
}

/***********************************************************
 * ��ʼ���˿�
 ***********************************************************/
void datetime_iic_port_init(void)
{
	MDATETIME_SCL_OUT("���");
	MDATETIME_SDA_OUT("���");
	
	MDATETIME_SDA_H("sda = 1");
	MDATETIME_SCL_H("scl = 1");
}

/**
 * ���ַΪDATETIME_I2C_SLAVE_ADDRESS7��iic�豸��addrд��data
 **/
int datetime_ByteWrite(u8 addr, u8 data)
{
	datetime_iic_start();
	datetime_iic_writebyte(DATETIME_I2C_SLAVE_ADDRESS7 << 1); datetime_iic_readACK();
	datetime_iic_writebyte(addr); datetime_iic_readACK();
	datetime_iic_writebyte(data); datetime_iic_readACK();
	datetime_iic_stop();
        
    return  0;
}
/*******************************************************************************
 * ��pBufferΪ��ʼ��ַ������ΪNumByteToRead������д�뵽
 * ��ַΪDATETIME_I2C_SLAVE_ADDRESS7��iic�豸��ַΪaddr�Ĵ�����
 *******************************************************************************/
int datetime_BytesWrite(u8* pBuffer, u8 WriteAddr, u8 NumByteToWrite)
{
	datetime_iic_start();
	datetime_iic_writebyte(DATETIME_I2C_SLAVE_ADDRESS7 << 1); datetime_iic_readACK();
	datetime_iic_writebyte(WriteAddr); datetime_iic_readACK();
	while(NumByteToWrite) {
		datetime_iic_writebyte(*pBuffer); 
		datetime_iic_readACK();
		
		pBuffer++;
		NumByteToWrite--;
	}
	datetime_iic_stop();
        
    return  0;
}

/**
 * �ӵ�ַΪDATETIME_I2C_SLAVE_ADDRESS7��iic�豸��addr����
 **/
int datetime_ByteRead(u8* pBuffer, u8 ReadAddr, u8 NumByteToRead)
{
	/** set address **/
	datetime_iic_start();
	datetime_iic_writebyte((DATETIME_I2C_SLAVE_ADDRESS7 << 1)); datetime_iic_readACK();
	datetime_iic_writebyte(ReadAddr); datetime_iic_readACK();
	
	/** read **/
	datetime_iic_start();
	datetime_iic_writebyte((DATETIME_I2C_SLAVE_ADDRESS7 << 1) | 1); datetime_iic_readACK();
	
	while(NumByteToRead) {
		*pBuffer = datetime_iic_readbyte();
		if(NumByteToRead > 1) {
			datetime_iic_writeACK();
		} else {
			datetime_iic_writeNCK();
		}
		pBuffer++;
		NumByteToRead--;
	}
	datetime_iic_stop();
        
	return  0;
}

const iicTxData_t datetimeCfg[] = {
	{0x00, 0x00},
	{0x01, 0x00},
};
#if	0
const iicTxData_t datetimeDeCfg[] = {
	{0x00, 0x20},
	{0x01, 0x00},
};
#endif
/*******************************************************************************
 * �ϵ�ʱ������ʱ 
 *******************************************************************************/
void datetimeDevice_init(void)
{
	int i = 0;
	for(i = 0; i < MTABSIZE(datetimeCfg);i++) {	
		datetime_ByteWrite(datetimeCfg[i].addr, datetimeCfg[i].data);
	}
}

/*******************************************************************************
 * �ϵ�ʱ��ʼֵ(�롢�֡�ʱ���ա����ڡ���/���͡���) 
 * ��(0-59)/��(0-59)/ʱ(0-23)/��(1-31)/����(0-6)/������(1-12)/��(0-99)ȫΪBCD���ʾ
 *******************************************************************************/
 
 #if	0
/*******************************************************************************
 * ������д��PCF8563
 *******************************************************************************/
void datetime_write(void)
{
	
}

/*******************************************************************************
 * ��PCF8563��������
 *******************************************************************************/
void datetime_read(void)
{
	
}
#endif
void datetime_init(void)
{
	
}

/*******************************************************************************/
void datetime_Deinit(void)
{
}

