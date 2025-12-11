#include "HALAL/Benchmarking_toolkit/HardfaultTrace.h"

extern GPIO_TypeDef* ports_hard_fault[];
extern uint16_t pins_hard_fault[];
extern uint8_t hard_fault_leds_count;

static void LED_Blink(uint32_t delay);
static void LED_init(void);
static void EnableGPIOClock(GPIO_TypeDef* port);
static void InitGPIO_Output(GPIO_TypeDef* port,uint16_t pin);
static void TIM_init(void);
static void delay_ms(uint32_t);

static void delay_ms(uint32_t ms){
    uint32_t start = LL_TIM_GetCounter(TIM2);
    uint64_t us = ms*10000;
    while((LL_TIM_GetCounter(TIM2) - start) < us){
       __NOP();
    }
}
static void LED_Blink(uint32_t delay){
    //PB0 blink
    for(int i = 0; i < hard_fault_leds_count;i++){
        LL_GPIO_TogglePin(ports_hard_fault[i],pins_hard_fault[i]);
    }
    delay_ms(delay);
}
static void LED_init(void){
      for(int i = 0; i < hard_fault_leds_count;i++){
        EnableGPIOClock(ports_hard_fault[i]);
        InitGPIO_Output(ports_hard_fault[i],pins_hard_fault[i]);
      } 
}
void Hard_fault_check(void){
    if(*(volatile uint32_t*)HF_FLASH_ADDR == HF_FLAG_VALUE){
        HardFaultLog log;
        memcpy(&log,(void*)HF_FLASH_ADDR,sizeof(HardFaultLog));
        LED_init();
        TIM_init();
        while(1){
           LED_Blink(200);
        }
        
    }
}
static void EnableGPIOClock(GPIO_TypeDef* port){
  if(port == GPIOA)  LL_AHB4_GRP1_EnableClock(LL_AHB4_GRP1_PERIPH_GPIOA);
        else if(port == GPIOB)  LL_AHB4_GRP1_EnableClock(LL_AHB4_GRP1_PERIPH_GPIOB);
        else if(port == GPIOC)  LL_AHB4_GRP1_EnableClock(LL_AHB4_GRP1_PERIPH_GPIOC);
        else if(port == GPIOD)  LL_AHB4_GRP1_EnableClock(LL_AHB4_GRP1_PERIPH_GPIOD);
        else if(port == GPIOE)  LL_AHB4_GRP1_EnableClock(LL_AHB4_GRP1_PERIPH_GPIOE);
        else if(port == GPIOF)  LL_AHB4_GRP1_EnableClock(LL_AHB4_GRP1_PERIPH_GPIOF);
        else if(port == GPIOG)  LL_AHB4_GRP1_EnableClock(LL_AHB4_GRP1_PERIPH_GPIOG);
        else if(port == GPIOJ)  LL_AHB4_GRP1_EnableClock(LL_AHB4_GRP1_PERIPH_GPIOJ);
        else if(port == GPIOK)  LL_AHB4_GRP1_EnableClock(LL_AHB4_GRP1_PERIPH_GPIOK);
}
static void InitGPIO_Output(GPIO_TypeDef* port,uint16_t pin){
    LL_GPIO_SetPinMode(port, pin, LL_GPIO_MODE_OUTPUT);
    LL_GPIO_SetPinOutputType(port, pin, LL_GPIO_OUTPUT_PUSHPULL);
    LL_GPIO_SetPinSpeed(port, pin, LL_GPIO_SPEED_FREQ_LOW);
    LL_GPIO_SetPinPull(port, pin, LL_GPIO_PULL_NO);
}

static void TIM_init(void){
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM2);

    LL_TIM_DisableCounter(TIM2);

    uint32_t psc = (SystemCoreClock / 1000000) - 1;//freq = 1000 hz
    LL_TIM_SetPrescaler(TIM2, psc);
    LL_TIM_SetAutoReload(TIM2, 0xFFFFFFFF);   
    LL_TIM_SetCounterMode(TIM2, LL_TIM_COUNTERMODE_UP);
    LL_TIM_SetCounter(TIM2, 0);

    LL_TIM_EnableCounter(TIM2);
}