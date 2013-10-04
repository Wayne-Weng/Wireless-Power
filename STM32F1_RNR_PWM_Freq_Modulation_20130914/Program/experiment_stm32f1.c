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

#define STATE_FREQ 0
#define STATE_DUTY 1

#define PWM_FREQ_MIN 648  // 110kHz
#define PWM_FREQ_MAX 351  // 205kHz
#define PWM_DUTY_MIN_205k PWM_FREQ_MAX/10  // 10%
#define PWM_DUTY_MAX_205k PWM_FREQ_MAX>>1  // 50%

#define FREQ TIM2->ARR

#define PWM1 TIM2->CCR2

/*=====================================================================================================*/
/*=====================================================================================================*/
void GPIO_Config( void );
void PWM_Config( void );
/*=====================================================================================================*/
/*=====================================================================================================*/
int main( void )
{
  u16 freq = PWM_FREQ_MIN;
	u16 duty = PWM_DUTY_MAX_205k;
	u8 state = STATE_FREQ;
	
  GPIO_Config();
  PWM_Config();

  while(1) {
    LED_G = ~LED_G;

    if(KEY_WU == 1){
			if(state == STATE_FREQ){
				freq++;
			  if(freq>=PWM_FREQ_MIN){
          freq = PWM_FREQ_MIN;
          LED_R = ~LED_R;
        }
        FREQ = freq-1;
		  	PWM1 = freq>>1;
      }else if(state == STATE_DUTY){
				duty++;
				if(duty>= PWM_DUTY_MAX_205k){
					duty = PWM_DUTY_MAX_205k;
					state = STATE_FREQ;
				}
				PWM1 = duty;
			}
			Delay_1ms(10);
    }else if(KEY_BO == 1){
			if(state == STATE_FREQ){
				freq--;
			  if(freq<=PWM_FREQ_MAX){
          freq = PWM_FREQ_MAX;
					state = STATE_DUTY;  
        }
        FREQ = freq-1;
			  PWM1 = freq>>1;
      }else if(state == STATE_DUTY){
				duty--;
				if(duty<= PWM_DUTY_MIN_205k){
					duty = PWM_DUTY_MIN_205k;
					LED_B = ~LED_B;
				}
				PWM1 = duty;
			}
			Delay_1ms(10);
		}
  
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
void PWM_Config( void )
{
  GPIO_InitTypeDef GPIO_Struct;
  TIM_TimeBaseInitTypeDef TIM_TimeBaseStruct;
  TIM_OCInitTypeDef TIM_OCInitStruct;

  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);

  /* TIM2 PWM1 PA1 */
  GPIO_Struct.GPIO_Pin = GPIO_Pin_1;
  GPIO_Struct.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Struct.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(GPIOA, &GPIO_Struct);

  TIM_DeInit(TIM2);

  /************************** PWM Output **************************************/
  /* 設定 TIM2 TIM3 TIM4 Time Base */
  TIM_TimeBaseStruct.TIM_Period = (u16)(PWM_FREQ_MIN-1);              // 週期 = 2.5ms, 400Hz
  TIM_TimeBaseStruct.TIM_Prescaler = (u16)(1-1);             // 除頻1 = 72M
  TIM_TimeBaseStruct.TIM_ClockDivision = TIM_CKD_DIV1;
  TIM_TimeBaseStruct.TIM_CounterMode = TIM_CounterMode_Up;    // 上數
  TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStruct);

  /* 設定 TIM2 TIM3 TIM4 OC */
  TIM_OCInitStruct.TIM_OCMode = TIM_OCMode_PWM1;              // 配置為 PWM1 模式
  TIM_OCInitStruct.TIM_OutputState = TIM_OutputState_Enable;  // 致能 OC
  TIM_OCInitStruct.TIM_Pulse = PWM_FREQ_MIN>>1;                 // 設置跳變值
  TIM_OCInitStruct.TIM_OCPolarity = TIM_OCPolarity_High;      // 當計數值小於 PWM_MOTOR_MIN 時為高電平
  TIM_OC2Init(TIM2, &TIM_OCInitStruct);                       // 初始化 TIM2 OC2
  TIM_OC2PreloadConfig(TIM2, TIM_OCPreload_Enable);           // 致能 TIM2 OC2 預裝載

  /* 啟動 */
  TIM_ARRPreloadConfig(TIM2, ENABLE);                         // 致能 TIM2 重載寄存器ARR
  TIM_Cmd(TIM2, ENABLE);                                      // 致能 TIM2

}
/*=====================================================================================================*/
/*=====================================================================================================*/
