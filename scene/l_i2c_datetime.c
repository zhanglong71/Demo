
#include "stm8s.h"
#include "CONST.h"
#include "ctype.h"
#include "macro.h"
#include "global.h"	
#include "debug.h"

#include "arch.h"
#include "l_i2c_datetime.h"

/**
 * 开始之后，scl/sda都处于L
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
 * 结束之后，scl/sda都处于H
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
 * 写1个byte(完成一个byte的发送后，SCL为L)
 ***********************************************************/
u8 datetime_iic_writebyte(u8 Data)
{
	u8 i;
	//u8 Ack_Bit;                  //应答信号
	for(i = 0; i < 8; i++) {
		if(Data & 0x80)MDATETIME_SDA_H("输出高");else MDATETIME_SDA_L("输出低");
		MDATETIME_SCL_H("数据有效");
		delaySpin(10);
		MDATETIME_SCL_L("准备下一个bit数据");
		delaySpin(6);
		Data <<= 1;
	}
	
	return Data;	
}

/***********************************************************
 * 读1个byte数据(读出一个byte后，SCL为L)
 ***********************************************************/
u8 datetime_iic_readbyte()
{
	u8 i;
	u8 Data = 0;
	
	MDATETIME_SDA_IN("sda置为输入");
	MDATETIME_SCL_OUT("scl置为输出");
	for(i = 0; i < 8; i++) {
		delaySpin(6);
		MDATETIME_SCL_H("准备读");
		delaySpin(6);
		Data <<= 1;
		if(MDATETIME_READ_SDA("读sda")){Data |= 0x01; } else { Data &= 0xfe; }
		MDATETIME_SCL_L("准备读下一个bit");
	}
	MDATETIME_SDA_OUT("sda置为输出");
	
	return Data;	
}
/***********************************************************
 * 读应答信号(在write一个byte之后)
 ***********************************************************/
u8 datetime_iic_readACK()
{
    u8 Ack_Bit;          			//应答信号
        
	MDATETIME_SDA_H("释放sda");		//释放IIC SDA总线为主器件接收从器件产生应答信号	
	delaySpin(5);
	MDATETIME_SCL_H("进入读周期");         	//第9个时钟周期
	delaySpin(12);
	Ack_Bit = MDATETIME_READ_SDA("读sda");	//读取应答信号
	MDATETIME_SCL_L("完成");
        
    return  Ack_Bit;
}

/***********************************************************
 * 写ACK应答信号(在read一个byte之后)
 ***********************************************************/
void datetime_iic_writeACK()
{
	MDATETIME_SDA_L("准备ACK");
	delaySpin(5);
	MDATETIME_SCL_H("ACK有效");
	delaySpin(12);
	MDATETIME_SCL_L("完成");
	delaySpin(5);
}
/***********************************************************
 * 写NCK应答信号(在read最后一个byte之后)
 ***********************************************************/
void datetime_iic_writeNCK()
{
	MDATETIME_SDA_H("准备NCK");
	nop();nop();
	MDATETIME_SCL_H("NCK有效");
	delaySpin(12);
	MDATETIME_SCL_L("完成");
	delaySpin(5);
}

/***********************************************************
 * 初始化端口
 ***********************************************************/
void datetime_iic_port_init(void)
{
	MDATETIME_SCL_OUT("输出");
	MDATETIME_SDA_OUT("输出");
	
	MDATETIME_SDA_H("sda = 1");
	MDATETIME_SCL_H("scl = 1");
}

/**
 * 向地址为DATETIME_I2C_SLAVE_ADDRESS7的iic设备的addr写入data
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
 * 将pBuffer为起始地址，长度为NumByteToRead的数据写入到
 * 地址为DATETIME_I2C_SLAVE_ADDRESS7的iic设备地址为addr的存贮区
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
 * 从地址为DATETIME_I2C_SLAVE_ADDRESS7的iic设备的addr读出
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
 * 上电时、唤醒时 
 *******************************************************************************/
void datetimeDevice_init(void)
{
	int i = 0;
	for(i = 0; i < MTABSIZE(datetimeCfg);i++) {	
		datetime_ByteWrite(datetimeCfg[i].addr, datetimeCfg[i].data);
	}
}

/*******************************************************************************
 * 上电时初始值(秒、分、时、日、星期、月/世纪、年) 
 * 秒(0-59)/分(0-59)/时(0-23)/日(1-31)/星期(0-6)/月世纪(1-12)/年(0-99)全为BCD码表示
 *******************************************************************************/
 
 #if	0
/*******************************************************************************
 * 将数据写入PCF8563
 *******************************************************************************/
void datetime_write(void)
{
	
}

/*******************************************************************************
 * 从PCF8563读出数据
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

