/*=====================================================================================================*/
/*=====================================================================================================*/
#include "stm32f1_system.h"
#include "stm32f1_adc.h"
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
#define PWM_FREQ_INIT 351  // 205kHz
#define PWM_DUTY_MIN_205k PWM_FREQ_MAX/10  // 10%
#define PWM_DUTY_MAX_205k PWM_FREQ_MAX>>1  // 50%

#define FREQ TIM2->ARR
#define PWM1 TIM2->CCR2

#define SET_POINT_5V 621

#define PID_INTERVAL 0.01

/*=====================================================================================================*/
/*=====================================================================================================*/
void GPIO_Config( void );
void PWM_Config( void );
void TIM_Config( void );
void Freq_Duty_Mod(s16 inputValue);
/*=====================================================================================================*/
/*=====================================================================================================*/

u16 ADC_AveTr[ADC_Channel] = {0};
/*=====================================================================================================*/
/*=====================================================================================================*/
float Kp = 15,Ki = 5,Kd = 2;

s16 error = 0;
s16 prev_error = 0;
float integral = 0;
float derivative = 0;
float output = 0;

/*=====================================================================================================*/
/*=====================================================================================================*/
int main( void )
{
  GPIO_Config();
	PWM_Config();
	ADC_Config();
  TIM_Config();
	
  while(1) {
		
		if(KEY_WU==1){
			while(KEY_WU==1);
			Kp+=1;
			if(Kp>=20) Kp = 0;
			Delay_10ms(2);
    }
		if(KEY_BO==1){
			while(KEY_BO==1);
			Kd+=0.1;
			if(Kd>=5) Kd = 0;
			Delay_10ms(2);
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
  TIM_TimeBaseStruct.TIM_Period = (u16)(PWM_FREQ_INIT-1);              // 週期
  TIM_TimeBaseStruct.TIM_Prescaler = (u16)(1-1);             // 除頻1 = 72M
  TIM_TimeBaseStruct.TIM_ClockDivision = TIM_CKD_DIV1;
  TIM_TimeBaseStruct.TIM_CounterMode = TIM_CounterMode_Up;    // 上數
  TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStruct);

  /* 設定 TIM2 TIM3 TIM4 OC */
  TIM_OCInitStruct.TIM_OCMode = TIM_OCMode_PWM1;              // 配置為 PWM1 模式
  TIM_OCInitStruct.TIM_OutputState = TIM_OutputState_Enable;  // 致能 OC
  TIM_OCInitStruct.TIM_Pulse = /*PWM_FREQ_INIT>>1*/PWM_DUTY_MIN_205k;                 // 設置跳變值
  TIM_OCInitStruct.TIM_OCPolarity = TIM_OCPolarity_High;      // 當計數值小於 PWM_MOTOR_MIN 時為高電平
  TIM_OC2Init(TIM2, &TIM_OCInitStruct);                       // 初始化 TIM2 OC2
  TIM_OC2PreloadConfig(TIM2, TIM_OCPreload_Enable);           // 致能 TIM2 OC2 預裝載

  /* 啟動 */
  TIM_ARRPreloadConfig(TIM2, ENABLE);                         // 致能 TIM2 重載寄存器ARR
  TIM_Cmd(TIM2, ENABLE);                                      // 致能 TIM2

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
void Freq_Duty_Mod(s16 inputValue)
{
	static u16 freq = PWM_FREQ_MAX;
	static u16 duty = PWM_DUTY_MIN_205k;
	static u8 state = STATE_DUTY;
  
	u16 count = 0;
	
	if(inputValue > 0){
		for(count=0;count<inputValue;count++){
			if(state == STATE_FREQ){
				freq++;
				if(freq>=PWM_FREQ_MIN){
					freq = PWM_FREQ_MIN;
					integral = 0;
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
			
		}
	}else if(inputValue < 0){
		for(count=0;count<((-1)*inputValue);count++){
			
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
					integral = 0;
					LED_B = ~LED_B;
				}
				PWM1 = duty;
			}
			
		}
	}
}

/*=====================================================================================================*/
/*=====================================================================================================*/
void TIM3_IRQHandler(void)
{
	ADC_Average(ADC_AveTr);
	
	error = SET_POINT_5V - ADC_AveTr[0];
	integral = integral + error*PID_INTERVAL;
  derivative = (error - prev_error)/PID_INTERVAL;
  output = Kp*error + Ki*integral + Kd*derivative;
	prev_error = error;
	
	Freq_Duty_Mod((s16)(output/100));
	
  LED_G = ~LED_G;
	
  TIM_ClearFlag(TIM3, TIM_FLAG_Update);
}
