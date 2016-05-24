#ifndef	__L_I2C_DATETIME_H__
#define	__L_I2C_DATETIME_H__


/* Includes ------------------------------------------------------------------*/
#include "stm8s.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define I2C_Speed              100000
//#define I2C2_SLAVE_ADDRESS7   (0x5A)
#define DATETIME_I2C_SLAVE_ADDRESS7   (0x51)
#define I2C2_OWN_ADDRESS7   (0x36)

/******************************************************************************/
#define	DATETIME_IIC_SDA_PORT	GPIOE
#define	DATETIME_IIC_SCL_PORT	GPIOE

#define	DATETIME_IIC_SDA_PIN		GPIO_PIN_7
#define	DATETIME_IIC_SCL_PIN		GPIO_PIN_6

#define	MDATETIME_SDA_H(x)	(GPIO_WriteHigh(DATETIME_IIC_SDA_PORT, DATETIME_IIC_SDA_PIN))
#define	MDATETIME_SDA_L(x)	(GPIO_WriteLow(DATETIME_IIC_SDA_PORT, DATETIME_IIC_SDA_PIN))

#define	MDATETIME_SCL_H(x)	(GPIO_WriteHigh(DATETIME_IIC_SCL_PORT, DATETIME_IIC_SCL_PIN))
#define	MDATETIME_SCL_L(x)	(GPIO_WriteLow(DATETIME_IIC_SCL_PORT, DATETIME_IIC_SCL_PIN))

#define	MDATETIME_SCL_OUT(x)	(GPIO_Init(DATETIME_IIC_SCL_PORT, DATETIME_IIC_SCL_PIN, GPIO_MODE_OUT_OD_LOW_FAST ))
#define	MDATETIME_SDA_OUT(x)	(GPIO_Init(DATETIME_IIC_SDA_PORT, DATETIME_IIC_SDA_PIN, GPIO_MODE_OUT_OD_LOW_FAST ))

#define	MDATETIME_SCL_IN(x) (GPIO_Init(DATETIME_IIC_SCL_PORT, DATETIME_IIC_SCL_PIN, GPIO_MODE_IN_FL_NO_IT ))
#define	MDATETIME_SDA_IN(x)	(GPIO_Init(DATETIME_IIC_SDA_PORT, DATETIME_IIC_SDA_PIN, GPIO_MODE_IN_FL_NO_IT ))

#define	MDATETIME_READ_SDA(x)	(GPIO_ReadInputPin(DATETIME_IIC_SDA_PORT, DATETIME_IIC_SDA_PIN))
#define	MDATETIME_READ_SCL(x)	(GPIO_ReadInputPin(DATETIME_IIC_SCL_PORT, DATETIME_IIC_SCL_PIN))
/******************************************************************************/	
void datetime_iic_port_init(void);

void datetimeDevice_init(void);
void datetime_init(void);
void datetime_Deinit(void);

int datetime_ByteWrite(u8 addr, u8 data);
int datetime_ByteRead(u8* pBuffer, u8 ReadAddr, u8 NumByteToRead);
int datetime_BytesWrite(u8* pBuffer, u8 WriteAddr, u8 NumByteToWrite);

/*******************************************************************************
 * 从地址为[2, 3, 4, 5, 6, 7, 8]的数据读出到(g_u8password[])
 * 对应为[sec, min, hou, day, week, mon/cen, year]
 * 以BCD码方式呈现
 *******************************************************************************/
#define	MDATETIME_READ(x)	datetime_ByteRead(g_u8password, 2, 7)
/*******************************************************************************
 * 将数据(g_u8password[])写入到地址为[2,3,4,5,6,7,8]
 *******************************************************************************/
#define	MDATETIME_WRITE(x)	datetime_BytesWrite(g_u8password, 2, 7)
/*******************************************************************************/
#define	Mdatetime_write(x)	datetime_BytesWrite(x, 2, 7)
#define	Mdatetime_read(x)	datetime_ByteRead(x, 2, 7)
/*******************************************************************************/

#endif

