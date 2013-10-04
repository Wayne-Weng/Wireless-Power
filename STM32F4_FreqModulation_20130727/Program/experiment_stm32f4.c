/*=====================================================================================================*/
/*=====================================================================================================*/
#include "stm32f4_system.h"
/*=====================================================================================================*/
/*=====================================================================================================*/
#define KEY   PBI(2)
#define LED_R PCO(15)
#define LED_G PCO(14)
#define LED_B PCO(13)

#define FREQ_MAX  336   //84*4
#define FREQ_MIN  840   //210*4 
#define FREQ_MAX_HALF 168   //FREQ_MAX/2
#define FREQ_MIN_HALF 420   //FREQ_MIN/2

#define PWM_DUTY  TIM3->CCR4

#define PWM_PERIOD TIM3->ARR


/*=====================================================================================================*/
/*=====================================================================================================*/
void GPIO_Config( void );
void PWM_Config( void );
/*=====================================================================================================*/
/*=====================================================================================================*/
int main( void )
{
	u32 freq = FREQ_MAX;

	SystemInit();
	GPIO_Config();
	PWM_Config();

  while(1) {
    LED_G = ~LED_G;

		while(KEY == 1) {

  PWM_PERIOD = freq;
	PWM_DUTY  = freq>>1;

			freq++;
			if(freq>=FREQ_MIN) {
				freq = FREQ_MAX;
				LED_R = ~LED_R;
			}
			Delay_10ms(2);
		}

    Delay_100ms(1);
	}
}
/*=====================================================================================================*/
/*=====================================================================================================*/
void GPIO_Config( void )
{
	GPIO_InitTypeDef GPIO_InitStruct;

  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOC, ENABLE);

  /* LED_R PC13 */  /* LED_G PC14 */  /* LED_B PC15 */
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOC, &GPIO_InitStruct);

  /* KEY PB2 */
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_2;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOB, &GPIO_InitStruct);

  LED_G = 1;
  LED_R = 1;
  LED_B = 1;
}
/*=====================================================================================================*/
/*=====================================================================================================*/
void PWM_Config( void )
{
	GPIO_InitTypeDef GPIO_InitStruct;
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStruct;
	TIM_OCInitTypeDef TIM_OCInitStruct;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

  GPIO_PinAFConfig(GPIOB, GPIO_PinSource1, GPIO_AF_TIM3);
  
	/* TIM3 PWM8 PB1 */
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_1;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOB, &GPIO_InitStruct);
  
	TIM_DeInit(TIM3);

/************************** PWM Output **************************************/

	/* 設定 TIM3 Time Base */
	TIM_TimeBaseStruct.TIM_Period = (u16)(FREQ_MIN-1);              // 週期 = 2.5ms, 400Hz
	TIM_TimeBaseStruct.TIM_Prescaler = (u16)(1-1);             // 除頻4 = 21M
	TIM_TimeBaseStruct.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStruct.TIM_CounterMode = TIM_CounterMode_Up;		// 上數
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStruct);
  
	/* 設定 TIM3 OC */
	TIM_OCInitStruct.TIM_OCMode = TIM_OCMode_PWM1;							// 配置為 PWM1 模式
	TIM_OCInitStruct.TIM_OutputState = TIM_OutputState_Enable;	// 致能 OC
	TIM_OCInitStruct.TIM_Pulse = FREQ_MIN_HALF;								    	// 設置跳變值
	TIM_OCInitStruct.TIM_OCPolarity = TIM_OCPolarity_High;			// 當計數值小於 PWM_MOTOR_MIN 時為高電平
	TIM_OC4Init(TIM3, &TIM_OCInitStruct);												// 初始化 TIM3 OC4
	TIM_OC4PreloadConfig(TIM3, TIM_OCPreload_Enable);						// 致能 TIM3 OC4 預裝載

	/* 啟動 */
	TIM_ARRPreloadConfig(TIM3, ENABLE);													// 致能 TIM3 重載寄存器ARR
	TIM_Cmd(TIM3, ENABLE);																			// 致能 TIM3

}
/*=====================================================================================================*/
/*=====================================================================================================*/
