#include "MockedDrivers/stm32h723xx_wrapper.h"

static RCC_TypeDef RCC_struct;
RCC_TypeDef *RCC = &RCC_struct;

static GPIO_TypeDef GPIOA_struct;
static GPIO_TypeDef GPIOB_struct;
static GPIO_TypeDef GPIOC_struct;
static GPIO_TypeDef GPIOD_struct;
static GPIO_TypeDef GPIOE_struct;
static GPIO_TypeDef GPIOF_struct;
static GPIO_TypeDef GPIOG_struct;
static GPIO_TypeDef GPIOH_struct;

GPIO_TypeDef *GPIOA = &GPIOA_struct;
GPIO_TypeDef *GPIOB = &GPIOB_struct;
GPIO_TypeDef *GPIOC = &GPIOC_struct;
GPIO_TypeDef *GPIOD = &GPIOD_struct;
GPIO_TypeDef *GPIOE = &GPIOE_struct;
GPIO_TypeDef *GPIOF = &GPIOF_struct;
GPIO_TypeDef *GPIOG = &GPIOG_struct;
GPIO_TypeDef *GPIOH = &GPIOH_struct;

static ADC_TypeDef ADC1_struct;
static ADC_TypeDef ADC2_struct;
static ADC_TypeDef ADC3_struct;

ADC_TypeDef *ADC1 = &ADC1_struct;
ADC_TypeDef *ADC2 = &ADC2_struct;
ADC_TypeDef *ADC3 = &ADC3_struct;
