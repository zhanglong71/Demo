
/* Define to prevent recursive inclusion ------------------------------------ */
#ifndef __L_I2C_TOUCH_H__
#define __L_I2C_TOUCH_H__

/* Includes ------------------------------------------------------------------*/
#include "stm8s.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define I2C_Speed              100000
#define I2C2_SLAVE_ADDRESS7   (0x5A)
#define I2C2_OWN_ADDRESS7   (0x36)

	
	// Global Constants
	//#define TOU_THRESH 	0x0B
	//#define TOU_THRESH	7
	//#define TOU_THRESH	5
	//#define TOU_THRESH	4
	#define TOU_THRESH	3
	
	//#define	REL_THRESH	0x07
	#define	REL_THRESH	2
/* ---------------------------------------------*/

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/

/* Exported functions ------------------------------------------------------- */
void I2C_TOUCH_Init(void);
void I2C_TOUCH_DeInit(void);
void mpr121_init(void);
int I2C_TOUCH_Read(u8* pBuffer, u8 ReadAddr, u8 NumByteToRead);
int I2C_TOUCH_ByteWrite(u8 addr, u8 data);
/******************************************************************************/
#define	IIC_SDA_PORT	GPIOE
#define	IIC_SCL_PORT	GPIOE
#define	IIC_SDA_PIN		GPIO_PIN_2
#define	IIC_SCL_PIN		GPIO_PIN_1

#define	MSDA_H	(GPIO_WriteHigh(IIC_SDA_PORT, IIC_SDA_PIN))
#define	MSDA_L	(GPIO_WriteLow(IIC_SDA_PORT, IIC_SDA_PIN))


#define	MSCL_OUT	(GPIO_Init(IIC_SCL_PORT, IIC_SCL_PIN, GPIO_MODE_OUT_OD_LOW_FAST ))
#define	MSDA_OUT	(GPIO_Init(IIC_SDA_PORT, IIC_SDA_PIN, GPIO_MODE_OUT_OD_LOW_FAST ))
#define	MSCL_IN (GPIO_Init(IIC_SCL_PORT, IIC_SCL_PIN, GPIO_MODE_IN_FL_NO_IT ))
#define	MSDA_IN	(GPIO_Init(IIC_SDA_PORT, IIC_SDA_PIN, GPIO_MODE_IN_FL_NO_IT ))
#define	MSDA	(GPIO_ReadInputPin(IIC_SDA_PORT, IIC_SDA_PIN))
#define	MSCL	(GPIO_ReadInputPin(IIC_SCL_PORT, IIC_SCL_PIN))

#define	MSCL_H	(GPIO_WriteHigh(IIC_SCL_PORT, IIC_SCL_PIN))
#define	MSCL_L	(GPIO_WriteLow(IIC_SCL_PORT, IIC_SCL_PIN))
/******************************************************************************/
#endif /* __I2C_EE_H */




