
/* Includes ------------------------------------------------------------------*/
#include "stm8s.h"
#include "CONST.h"
#include "ctype.h"
#include "macro.h"
#include "global.h"
#include "debug.h"

#include "arch.h"

//#include "led.h"
#include "stm8s_i2c.h"
#include "l_i2c_touch.h"

const iicTxData_t iicTouchCfg[] = {
	// MPR121 Register Defines
#if	1
	#define	ELE0_T	0x41
	#define	ELE0_R	0x42
	#define	ELE1_T	0x43
	#define	ELE1_R	0x44
	#define	ELE2_T	0x45
	#define	ELE2_R	0x46
	#define	ELE3_T	0x47
	#define	ELE3_R	0x48
	#define	ELE4_T	0x49
	#define	ELE4_R	0x4A
	#define	ELE5_T	0x4B
	#define	ELE5_R	0x4C
	#define	ELE6_T	0x4D
	#define	ELE6_R	0x4E
	#define	ELE7_T	0x4F
	#define	ELE7_R	0x50
	#define	ELE8_T	0x51
	#define	ELE8_R	0x52
	#define	ELE9_T	0x53
	#define	ELE9_R	0x54
	#define	ELE10_T	0x55
	#define	ELE10_R	0x56
	#define	ELE11_T	0x57
	#define	ELE11_R	0x58
#endif
	//reset MPR121 if not reset correctly
	{0x80, 0x63},	//soft reset
	{0x73, 0xff},
	{0x74, 0xff},
	{0x76, 0xff},
	{0x77, 0xff},
	{0x75, 0xff},
	
	{0x81, 0x00},
	{0x82, 0x00},
	{0x83, 0x00},
	{0x84, 0x00},
	
	// Section A touchpd baseline, filter rising. This group controls filtering when data is > baseline.
	{0x2b, 0x01},	//0xff//max half delta rising
	{0x2c, 0x01},	//0xff//noise half delta rising
	{0x2d, 0x00},	//0 noise count limit rising
	{0x2e, 0x00},	//delay limit rising
	// Section B failing  This group controls filtering when data is < baseline.
	{0x2f, 0x01},
	{0x30, 0x01},
	{0x31, 0xFF},
	{0x32, 0x00},
	//...touched
	{0x33, 0x00},	//Noise half delta touched
	{0x34, 0x00},	//Noise counts touched
	{0x35, 0x00},	//Filter delay touched
	
	{0x36, 0x0f},
	{0x37, 0x0f},
	{0x38, 0x00},
	{0x39, 0x00},
	{0x3a, 0x01},
	{0x3b, 0x01},
	{0x3c, 0xff},
	{0x3d, 0xff},
	{0x3e, 0x00},
	{0x3f, 0x00},
	{0x40, 0x00},
	// Section C
  	// This group sets touch and release thresholds for each electrode
	{ELE0_T, TOU_THRESH},
	{ELE0_R, REL_THRESH},
	{ELE1_T, TOU_THRESH},
	{ELE1_R, REL_THRESH},
	{ELE2_T, TOU_THRESH},
	{ELE2_R, REL_THRESH},
	{ELE3_T, TOU_THRESH},
	{ELE3_R, REL_THRESH},
	{ELE4_T, TOU_THRESH},
	{ELE4_R, REL_THRESH},
	{ELE5_T, TOU_THRESH},
	{ELE5_R, REL_THRESH},
	{ELE6_T, TOU_THRESH},
	{ELE6_R, REL_THRESH},
	{ELE7_T, TOU_THRESH},
	{ELE7_R, REL_THRESH},
	{ELE8_T, TOU_THRESH},
	{ELE8_R, REL_THRESH},
	{ELE9_T, TOU_THRESH},
	{ELE9_R, REL_THRESH},
	{ELE10_T, TOU_THRESH},
	{ELE10_R, REL_THRESH},
	{ELE11_T, TOU_THRESH},
	{ELE11_R, REL_THRESH},
	
	{0x59, 0x02},
	{0x5a, 0x01},
	{0x5b, 0x00},
	{0x5c, 0x10},
	{0x5d, 0x24},
  	
  	// Section F
  	// Enable Auto Config and auto Reconfig	
	{0x7B, 0x0B},
	{0x7D, 0xc8},
	{0x7E, 0x82},
	{0x7F, 0xb4},
	
	{0x5E, 0xbc},
};

/*******************************************************************************
 * IIC GPIO实现
 * 
 *******************************************************************************/
 #define	CIIC_GPIO	1
//#undef  CIIC_GPIO
#if		CIIC_GPIO
void iic_start()
{
	MSDA_H;
	delaySpin(6);
	MSCL_H;
	delaySpin(13);
	MSDA_L;
	delaySpin(6);
	MSCL_L;
}

void iic_stop()
{
	MSDA_L;
	nop();nop();
	MSCL_H;
	nop();nop();
	MSDA_H;
	nop();nop();
}

/***********************************************************
 * 写1个byte数据
 ***********************************************************/
u8 iic_writebyte(u8 Data)
{
	u8 i;
	//u8 Ack_Bit;                  //应答信号
	for(i = 0; i < 8; i++) {
		if(Data & 0x80)MSDA_H;else MSDA_L;
		MSCL_H;
		//nop();nop();nop();nop();
		delaySpin(10);
		MSCL_L;
		delaySpin(6);
		Data <<= 1;
	}
	
	return Data;	
}

/***********************************************************
 * 读1个byte数据
 ***********************************************************/
u8 iic_readbyte()
{
	u8 i;
	u8 Data = 0;
	
	//MSDA_H;
	MSDA_IN;
	MSCL_OUT;
	for(i = 0; i < 8; i++) {
		delaySpin(6);
		MSCL_H;
		delaySpin(6);
		Data <<= 1;
		if(MSDA){Data |= 0x01; } else { Data &= 0xfe; }
		MSCL_L;
	}
	MSDA_OUT;
	
	return Data;	
}

u8 iic_readACK()
{
    u8 Ack_Bit;                    //应答信号
        
	MSDA_H;		                //释放IIC SDA总线为主器件接收从器件产生应答信号	
	delaySpin(5);
	MSCL_H;                     //第9个时钟周期
	delaySpin(12);
	Ack_Bit = MSDA;		        //读取应答信号
	MSCL_L;
        
    return  Ack_Bit;
}

void iic_writeACK()
{
	MSDA_L;
	delaySpin(5);
	MSCL_H;
	delaySpin(12);
	MSCL_L;
	delaySpin(5);
}
void iic_writeNCK()
{
	MSDA_H;
	nop();nop();
	MSCL_H;
	delaySpin(12);
	MSCL_L;
	delaySpin(5);
}

#endif
/*******************************************************************************
* Function Name  : I2C_TOUCHInit
* Description    : Initializes peripherals used by the I2C touch driver.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void I2C_TOUCH_Init(void)
{
	#if	CIIC_GPIO
	GPIO_Init(IIC_SDA_PORT, IIC_SDA_PIN, GPIO_MODE_OUT_OD_HIZ_SLOW );
	GPIO_Init(IIC_SCL_PORT, IIC_SCL_PIN, GPIO_MODE_OUT_OD_HIZ_SLOW ); 
	//delaySpin_ms(1);
	MSDA_H;
	MSCL_H;
	#else
	u8 Input_Clock = 0x0;
	/* Get system clock frequency */

	I2C_Cmd(DISABLE);
	I2C_DeInit();
	Input_Clock = CLK_GetClockFreq()/1000000;
	/* I2C Peripheral Enable */
	I2C_Cmd(ENABLE);
	/* Apply I2C configuration after enabling it */
	I2C_Init(I2C_Speed, I2C2_OWN_ADDRESS7, I2C_DUTYCYCLE_2,\
	I2C_ACK_CURR, I2C_ADDMODE_7BIT, Input_Clock);
	#endif
}

/*********************************************************************
 * 进入低功耗状态。(这些端口设为浮空输入端口) 
 *********************************************************************/
void I2C_TOUCH_DeInit(void)
{
	#if		CIIC_GPIO
	
	GPIO_Init(IIC_SDA_PORT, IIC_SDA_PIN, GPIO_MODE_OUT_OD_HIZ_SLOW );
	GPIO_Init(IIC_SCL_PORT, IIC_SCL_PIN, GPIO_MODE_OUT_OD_HIZ_SLOW ); 
	//delaySpin_ms(1);
	MSDA_H;
	MSCL_H;
	
	//GPIO_Init(IIC_SCL_PORT, IIC_SCL_PIN, GPIO_MODE_IN_FL_NO_IT );
	//GPIO_Init(IIC_SDA_PORT, IIC_SDA_PIN, GPIO_MODE_IN_FL_NO_IT );
	#else
	CLK_PeripheralClockConfig(CLK_PERIPHERAL_I2C,DISABLE);
	#endif
}
 /*******************************************************************************
* Function Name  : I2C_TOUCH_ByteWrite
* Description    : Writes one byte to the touch driver.
* Input          : - pBuffer : pointer to the buffer  containing the data to be 
*                    written to the EEPROM.
*                  - addr : device's internal address to write to.
* Output         : None
* Return         : None
*******************************************************************************/
int I2C_TOUCH_ByteWrite(u8 addr, u8 data)
{
#if		CIIC_GPIO
	iic_start();
	iic_writebyte(I2C2_SLAVE_ADDRESS7 << 1);iic_readACK();
	iic_writebyte(addr);iic_readACK();
	iic_writebyte(data);iic_readACK();
	iic_stop();
#else
	/* Send STRAT condition */
	I2C_GenerateSTART(ENABLE);

	/* Test on EV5 and clear it */
	while(!I2C_CheckEvent(I2C_EVENT_MASTER_START_SENT));
	//while(!I2C_CheckEvent(I2C_EVENT_MASTER_MODE_SELECT));

	/* Send EEPROM address for write */
	//I2C_Send7bitAddress(EEPROM_ADDRESS, I2C_DIRECTION_TX);
	I2C_Send7bitAddress(I2C2_SLAVE_ADDRESS7 << 1, I2C_DIRECTION_TX);

	/* Test on EV6 and clear it */
	while(!I2C_CheckEvent(I2C_EVENT_MASTER_ADDRESS_ACKED));
	//while(!I2C_CheckEvent(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));
	I2C_ClearFlag(I2C_FLAG_ADDRESSSENTMATCHED);

	I2C_SendData(addr); /* LSB */
	/* Test on EV8 and clear it */
	while (!I2C_CheckEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTING));

	/* Send the byte to be written */
	I2C_SendData(data);

	/* Test on EV8 and clear it */
	while(!I2C_CheckEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED));

	/* Send STOP condition */
	I2C_GenerateSTOP(ENABLE);
#endif
	return  0;
}

/*******************************************************************************
* Function Name  : I2C_TOUCH_Read
* Description    : Reads a block of data from the MPR121.
* Input          : - pBuffer : pointer to the buffer that receives the data read 
*                    from the iic
*                  - ReadAddr : MPR121 internal address to read from.
*                  - NumByteToRead : number of bytes to read from the EEPROM.
* Output         : *pBuffer
* Return         : 0 - no data
 *				   1 - valid
*******************************************************************************/
int I2C_TOUCH_Read(u8* pBuffer, u8 ReadAddr, u8 NumByteToRead)
{
#if		CIIC_GPIO

	//unsigned short wait;
	iic_start();
	iic_writebyte((I2C2_SLAVE_ADDRESS7 << 1));iic_readACK();
	iic_writebyte(ReadAddr);iic_readACK();
	
	iic_start();
	iic_writebyte((I2C2_SLAVE_ADDRESS7 << 1) | 1);iic_readACK();
	
	//wait = 0;
	while(NumByteToRead) {
		*pBuffer = iic_readbyte();
		if(NumByteToRead > 1) {
			iic_writeACK();
		} else {
			iic_writeNCK();
		}
		pBuffer++;
		NumByteToRead--;
	}
	iic_stop();
#else
	unsigned short wait;
	
	if((NumByteToRead <= 0) || (NumByteToRead > 4))
		return	0;
	/* While the bus is busy */
	while(I2C_GetFlagStatus(I2C_FLAG_BUSBUSY)){if(wait++ >= 1000) {return 0;} /* LED1_Toggle(); */ }

	/* Generate start & wait event detection */
	I2C_GenerateSTART(ENABLE);
	/* Test on EV5 and clear it */
    wait = 0;
	while (!I2C_CheckEvent(I2C_EVENT_MASTER_START_SENT)) {if(wait++ >= 1000) {return 0;}/* LED2_Toggle(); */}
	//while (!I2C_CheckEvent(I2C_EVENT_MASTER_MODE_SELECT)) {if(wait++ >= 1000) {return 0;} /* LED2_Toggle() */; }
        
	/* Send slave Address in write direction & wait detection event */
	//I2C_Send7bitAddress(EEPROM_ADDRESS, I2C_DIRECTION_TX);
	I2C_Send7bitAddress(I2C2_SLAVE_ADDRESS7 << 1, I2C_DIRECTION_TX);
	/* Test on EV6 and clear it */
	wait = 0;
	while (!I2C_CheckEvent(I2C_EVENT_MASTER_ADDRESS_ACKED)){if(wait++ >= 1000) {return	0;} /* LED3_Toggle(); */}
	//while (!I2C_CheckEvent(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED)) {if(wait++ >= 1000) {return	0;}/* LED3_Toggle(); */}
	I2C_ClearFlag(I2C_FLAG_ADDRESSSENTMATCHED);

	I2C_SendData(ReadAddr); 
	/* Test on EV8 and clear it */
	while (!I2C_CheckEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED)){if(wait++ >= 1000) {return	0;} /* LED4_Toggle(); */ } 

	/* Send STRAT condition a second time */
	I2C_GenerateSTART(ENABLE);
	/* Test on EV5 and clear it */
	while (!I2C_CheckEvent(I2C_EVENT_MASTER_START_SENT)){if(wait++ >= 1000) {return	0;} /* LED5_Toggle(); */ } 
	//while (!I2C_CheckEvent(I2C_EVENT_MASTER_MODE_SELECT)) /** LED5_Toggle() **/;

	/* Send slave Address in read direction & wait event */
	I2C_Send7bitAddress(I2C2_SLAVE_ADDRESS7 << 1, I2C_DIRECTION_RX);
	/* Test on EV6 and clear it */
	while (!I2C_CheckEvent(I2C_EVENT_MASTER_ADDRESS_ACKED)){if(wait++ >= 1000) {return	0;} /* LED6_Toggle(); */} 
	//while (!I2C_CheckEvent(I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED)) /**LED6_Toggle()**/;
	I2C_ClearFlag(I2C_FLAG_ADDRESSSENTMATCHED);

	wait = 0;
	/* While there is data to be read */
	while(NumByteToRead)
	{
		if(wait++ >= 1000) {return	0;} /* LED7_Toggle(); */
		if(NumByteToRead == 1)
		{
			/* Disable Acknowledgement */
			I2C_AcknowledgeConfig(I2C_ACK_NONE);

			/* Send STOP Condition */
			I2C_GenerateSTOP(ENABLE);
		}

		/* Test on EV7 and clear it */
		if(I2C_CheckEvent(I2C_EVENT_MASTER_BYTE_RECEIVED))
		{
			/* Read a byte from the EEPROM */
			*pBuffer = I2C_ReceiveData();

			/* Point to the next location where the byte read will be saved */
			pBuffer++;

			/* Decrement the read bytes counter */
			NumByteToRead--;
		}
	}

	/* Enable Acknowledgement to be ready for another reception */
	I2C_AcknowledgeConfig(I2C_ACK_CURR);
#endif
	return  1;
}

/*******************************************************************************/
void mpr121_init(void)
{
	int i = 0;
	for(i = 0; i < sizeof(iicTouchCfg)/sizeof(iicTouchCfg[0]);i++) {	
		I2C_TOUCH_ByteWrite(iicTouchCfg[i].addr, iicTouchCfg[i].data);
	}
}
