
#include "stm8s_iwdg.h"
#include "stm8s_rst.h"
#include "iwdg.h"
//#include "led.h"

void IWDG_Configuration(void)
{
  /* Enable the IWDG*/
  IWDG_Enable();
  /* Enable the access to the IWDG registers*/
  IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
  /* Fixe IWDG Reset period */
  IWDG_SetPrescaler(IWDG_Prescaler_256);//250hz
  //IWDG_SetPrescaler(IWDG_Prescaler_64);//???hz
  IWDG_SetReload(0xFA);//1s内喂狗 250次递减 fa递减到00 最大的时间是1s
  /* Refresh IWDG */
  IWDG_ReloadCounter();
}

void Test_IWDGReset(void)
{
  FlagStatus IwdgFlag;

  /*Get IWDG Reset Status */ 
  IwdgFlag = RST_GetFlagStatus(RST_FLAG_IWDGF);
  /* Test if a IWDG Reset has occured */
  if (IwdgFlag)
  {
   //LED_onoff();
    /* Clear IWDGF Flag */
    RST_ClearFlag(RST_FLAG_IWDGF);
    
   }
}
