#include "stm32h723xx_wrapper.h"

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

static DMA_Stream_TypeDef DMA1_Stream0_struct;
static DMA_Stream_TypeDef DMA1_Stream1_struct;
static DMA_Stream_TypeDef DMA1_Stream2_struct;
static DMA_Stream_TypeDef DMA1_Stream3_struct;
static DMA_Stream_TypeDef DMA1_Stream4_struct;
static DMA_Stream_TypeDef DMA1_Stream5_struct;
static DMA_Stream_TypeDef DMA1_Stream6_struct;
static DMA_Stream_TypeDef DMA1_Stream7_struct;
static DMA_Stream_TypeDef DMA2_Stream0_struct;
static DMA_Stream_TypeDef DMA2_Stream1_struct;
static DMA_Stream_TypeDef DMA2_Stream2_struct;
static DMA_Stream_TypeDef DMA2_Stream3_struct;
static DMA_Stream_TypeDef DMA2_Stream4_struct;
static DMA_Stream_TypeDef DMA2_Stream5_struct;
static DMA_Stream_TypeDef DMA2_Stream6_struct;
static DMA_Stream_TypeDef DMA2_Stream7_struct;

DMA_Stream_TypeDef *DMA1_Stream0 = &DMA1_Stream0_struct;
DMA_Stream_TypeDef *DMA1_Stream1 = &DMA1_Stream1_struct;
DMA_Stream_TypeDef *DMA1_Stream2 = &DMA1_Stream2_struct;
DMA_Stream_TypeDef *DMA1_Stream3 = &DMA1_Stream3_struct;
DMA_Stream_TypeDef *DMA1_Stream4 = &DMA1_Stream4_struct;
DMA_Stream_TypeDef *DMA1_Stream5 = &DMA1_Stream5_struct;
DMA_Stream_TypeDef *DMA1_Stream6 = &DMA1_Stream6_struct;
DMA_Stream_TypeDef *DMA1_Stream7 = &DMA1_Stream7_struct;
DMA_Stream_TypeDef *DMA2_Stream0 = &DMA2_Stream0_struct;
DMA_Stream_TypeDef *DMA2_Stream1 = &DMA2_Stream1_struct;
DMA_Stream_TypeDef *DMA2_Stream2 = &DMA2_Stream2_struct;
DMA_Stream_TypeDef *DMA2_Stream3 = &DMA2_Stream3_struct;
DMA_Stream_TypeDef *DMA2_Stream4 = &DMA2_Stream4_struct;
DMA_Stream_TypeDef *DMA2_Stream5 = &DMA2_Stream5_struct;
DMA_Stream_TypeDef *DMA2_Stream6 = &DMA2_Stream6_struct;
DMA_Stream_TypeDef *DMA2_Stream7 = &DMA2_Stream7_struct;

static SPI_TypeDef SPI1_struct;
static SPI_TypeDef SPI2_struct;
static SPI_TypeDef SPI3_struct;
static SPI_TypeDef SPI4_struct;
static SPI_TypeDef SPI5_struct;
static SPI_TypeDef SPI6_struct;

SPI_TypeDef *SPI1 = &SPI1_struct;
SPI_TypeDef *SPI2 = &SPI2_struct;
SPI_TypeDef *SPI3 = &SPI3_struct;
SPI_TypeDef *SPI4 = &SPI4_struct;
SPI_TypeDef *SPI5 = &SPI5_struct;
SPI_TypeDef *SPI6 = &SPI6_struct;
