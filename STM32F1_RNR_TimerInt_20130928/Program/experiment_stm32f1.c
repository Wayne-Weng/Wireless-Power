/*=====================================================================================================*/
/*=====================================================================================================*/
#include "stm32f1_system.h"
/*=====================================================================================================*/
/*=====================================================================================================*/
#define LED_R   PCO(13)
#define LED_G   PCO(14)
#define LED_B   PCO(15)

#define KEY_WU  PAI(0)
#define KEY_BO  PBI(2)

/*=====================================================================================================*/
/*=====================================================================================================*/
void GPIO_Config( void );
void TIM_Config( void );
/*=====================================================================================================*/
/*=====================================================================================================*/
int main( void )
{
  
  GPIO_Config();
  TIM_Config();

  while(1) {
    
  }
}
/*=====================================================================================================*/
/*=====================================================================================================*/
void GPIO_Config( void )
{
  GPIO_InitTypeDef GPIO_InitStruct;

  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC, ENABLE);
  
  /* PC13 LED_B */	/* PC14 LED_G */	/* PC15 LED_R */
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOC, &GPIO_InitStruct);

  /* KEY_WU PA0 */	/* KEY_BO PB2 */
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOA, &GPIO_InitStruct);
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_2;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOB, &GPIO_InitStruct);

  LED_R = 1;
  LED_G = 1;
  LED_B = 1;
}
/*=====================================================================================================*/
/*=====================================================================================================*/
void TIM_Config( void )
{
  TIM_TimeBaseInitTypeDef TIM_TimeBaseStruct;
  NVIC_InitTypeDef NVIC_InitStruct;
	
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

  TIM_DeInit(TIM3);

  /************************** PWM Output **************************************/
  /* 設定 TIM3 Time Base */
  TIM_TimeBaseStruct.TIM_Period = (u16)(1000-1);              // 週期 = 10ms, 100Hz
  TIM_TimeBaseStruct.TIM_Prescaler = (u16)(720-1);             // 除頻720 = 100kHz
  TIM_TimeBaseStruct.TIM_ClockDivision = TIM_CKD_DIV1;
  TIM_TimeBaseStruct.TIM_CounterMode = TIM_CounterMode_Up;    // 上數
  TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStruct);

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
	NVIC_InitStruct.NVIC_IRQChannel = TIM3_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStruct);
	
  /* 啟動 */
  TIM_ARRPreloadConfig(TIM3, ENABLE);                         // 致能 TIM3 重載寄存器ARR
	TIM_ClearFlag(TIM3, TIM_FLAG_Update);
	TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
  TIM_Cmd(TIM3, ENABLE);                                      // 致能 TIM3

}
/*=====================================================================================================*/
/*=====================================================================================================*/
void TIM3_IRQHandler(void){
	LED_R = ~LED_R;
  LED_G = ~LED_G;
  LED_B = ~LED_B;
  TIM_ClearFlag(TIM3, TIM_FLAG_Update);
}
