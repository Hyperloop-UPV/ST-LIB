#include "HALAL/Benchmarking_toolkit/HardfaultTrace.h"
#define REPS 100000
extern GPIO_TypeDef* ports_hard_fault[];
extern uint16_t pins_hard_fault[];
extern uint8_t hard_fault_leds_count;

static void LED_Blink();
static void LED_init(void);
static void EnableGPIOClock(GPIO_TypeDef* port);
static void InitGPIO_Output(GPIO_TypeDef* port,uint16_t pin);
static void delay();

__attribute__((optimize("O0")))
static void delay(){
    for(volatile uint64_t i = 0; i < REPS; i++){
       __NOP();
    }
}
static void LED_Blink(){
    for(volatile int i = 0; i < hard_fault_leds_count;i++){
        LL_GPIO_TogglePin(ports_hard_fault[i],pins_hard_fault[i]);
    }
    delay();
}
static void LED_init(void){
      for(int i = 0; i < hard_fault_leds_count;i++){
        EnableGPIOClock(ports_hard_fault[i]);
        InitGPIO_Output(ports_hard_fault[i],pins_hard_fault[i]);
      } 
}
void Hard_fault_check(void){
    if(*(volatile uint32_t*)HF_FLASH_ADDR == HF_FLAG_VALUE){
        volatile HardFaultLog log;
        memcpy(&log,(void*)HF_FLASH_ADDR,sizeof(HardFaultLog));
        #ifdef DEBUG
            __asm("bkpt 1");
        #endif
        LED_init();
        while(1){
           LED_Blink();
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

