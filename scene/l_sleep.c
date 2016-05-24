
#include "stm8s.h"
#include "CONST.h"
#include "ctype.h"
#include "macro.h"
#include "global.h"	
#include "debug.h"

#include "stm8s_awu.h"
#include "stm8s_tim3.h"

#include "l_sleep.h"

/**
  * @brief  Configure the AWU time base to 12s
  * @param  None
  * @retval None
  */
void AWU_Config(void)
{
    /* Initialization of AWU */
    /* LSI calibration for accurate auto wake up time base*/
    AWU_LSICalibrationConfig(LSIMeasurment());
    
    //slow Active halt
    //CLK_SlowActiveHaltWakeUpCmd(ENABLE);

	
    AWU_Cmd(ENABLE);
    /* The delay corresponds to the time we will stay in Halt mode */
    //AWU_Init(AWU_TIMEBASE_12S);
    AWU_Init(AWU_TIMEBASE_2S);
}

/**
  * @brief  Measure the LSI frequency using timer IC1 and update the calibration registers.
  * @note   It is recommended to use a timer clock frequency of at least 10MHz in order 
	*         to obtain a better in the LSI frequency measurement.
	* @param  None
  * @retval None
  */
uint32_t LSIMeasurment(void)
{

	uint32_t lsi_freq_hz = 0x0;
	uint32_t fmaster = 0x0;
	uint16_t ICValue1 = 0x0;
	uint16_t ICValue2 = 0x0;

	/* Get master frequency */
	fmaster = CLK_GetClockFreq();

	/* Enable the LSI measurement: LSI clock connected to timer Input Capture 1 */
	AWU->CSR |= AWU_CSR_MSR;

	/* Measure the LSI frequency with TIMER Input Capture 1 */

	/* Capture only every 8 events!!! */
	/* Enable capture of TI1 */
	TIM3_ICInit(TIM3_CHANNEL_1, TIM3_ICPOLARITY_RISING, TIM3_ICSELECTION_DIRECTTI, TIM3_ICPSC_DIV8, 0);

	/* Enable TIM3 */
	TIM3_Cmd(ENABLE);

	/* wait a capture on cc1 */
	while ((TIM3->SR1 & TIM3_FLAG_CC1) != TIM3_FLAG_CC1);
	/* Get CCR1 value*/
	ICValue1 = TIM3_GetCapture1();
	TIM3_ClearFlag(TIM3_FLAG_CC1);

	/* wait a capture on cc1 */
	while ((TIM3->SR1 & TIM3_FLAG_CC1) != TIM3_FLAG_CC1);
	/* Get CCR1 value*/
	ICValue2 = TIM3_GetCapture1();
	TIM3_ClearFlag(TIM3_FLAG_CC1);

	/* Disable IC1 input capture */
	TIM3->CCER1 &= (uint8_t)(~TIM3_CCER1_CC1E);
	/* Disable timer3 */
	TIM3_Cmd(DISABLE);

	/* Compute LSI clock frequency */
	lsi_freq_hz = (8 * fmaster) / (ICValue2 - ICValue1);

	/* Disable the LSI measurement: LSI clock disconnected from timer Input Capture 1 */
	AWU->CSR &= (uint8_t)(~AWU_CSR_MSR);

	return (lsi_freq_hz);
}

