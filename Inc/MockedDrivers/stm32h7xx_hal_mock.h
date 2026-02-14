#pragma once

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum { RESET = 0U, SET = !RESET } FlagStatus, ITStatus;
typedef enum { DISABLE = 0U, ENABLE = !DISABLE } FunctionalState;

typedef int32_t IRQn_Type;
enum {
    TIM2_IRQn = 28,
    TIM3_IRQn = 29,
    TIM4_IRQn = 30,
    TIM5_IRQn = 50,
    SPI1_IRQn = 35,
    SPI2_IRQn = 36,
    SPI3_IRQn = 51,
    SPI4_IRQn = 84,
    SPI5_IRQn = 85,
    SPI6_IRQn = 86,
    DMA1_Stream0_IRQn = 11,
    DMA1_Stream1_IRQn = 12,
    DMA1_Stream2_IRQn = 13,
    DMA1_Stream3_IRQn = 14,
    DMA1_Stream4_IRQn = 15,
    DMA1_Stream5_IRQn = 16,
    DMA1_Stream6_IRQn = 17,
    DMA1_Stream7_IRQn = 47,
    DMA2_Stream0_IRQn = 56,
    DMA2_Stream1_IRQn = 57,
    DMA2_Stream2_IRQn = 58,
    DMA2_Stream3_IRQn = 59,
    DMA2_Stream4_IRQn = 60,
    DMA2_Stream5_IRQn = 61,
    DMA2_Stream6_IRQn = 62,
    DMA2_Stream7_IRQn = 63,
    TIM6_DAC_IRQn = 54,
    TIM7_IRQn = 55,
    TIM8_BRK_TIM12_IRQn = 43,
    TIM8_UP_TIM13_IRQn = 44,
    TIM8_TRG_COM_TIM14_IRQn = 45,
    TIM1_UP_IRQn = 25,
    TIM15_IRQn = 68,
    TIM16_IRQn = 69,
    TIM17_IRQn = 70,
    TIM23_IRQn = 71,
    TIM24_IRQn = 72,
};

typedef struct {
    volatile uint32_t D1CFGR;
    volatile uint32_t D2CFGR;
    volatile uint32_t APB1LENR;
    volatile uint32_t APB1HENR;
    volatile uint32_t APB2ENR;
    volatile uint32_t APB4ENR;
    volatile uint32_t AHB4ENR;
} RCC_TypeDef;
extern RCC_TypeDef* RCC;
extern uint32_t SystemCoreClock;

#define RCC_D1CFGR_HPRE_Msk (0xFU << 0U)
#define RCC_D2CFGR_D2PPRE1_Pos 4U
#define RCC_D2CFGR_D2PPRE1_Msk (0x7U << RCC_D2CFGR_D2PPRE1_Pos)
#define RCC_D2CFGR_D2PPRE1 RCC_D2CFGR_D2PPRE1_Msk
#define RCC_D2CFGR_D2PPRE2_Pos 8U
#define RCC_D2CFGR_D2PPRE2_Msk (0x7U << RCC_D2CFGR_D2PPRE2_Pos)
#define RCC_D2CFGR_D2PPRE2 RCC_D2CFGR_D2PPRE2_Msk
#define RCC_HCLK_DIV1 0x00000000U

#define RCC_APB1LENR_TIM2EN (1U << 0)
#define RCC_APB1LENR_TIM3EN (1U << 1)
#define RCC_APB1LENR_TIM4EN (1U << 2)
#define RCC_APB1LENR_TIM5EN (1U << 3)
#define RCC_APB1LENR_TIM6EN (1U << 4)
#define RCC_APB1LENR_TIM7EN (1U << 5)
#define RCC_APB1LENR_TIM12EN (1U << 6)
#define RCC_APB1LENR_TIM13EN (1U << 7)
#define RCC_APB1LENR_TIM14EN (1U << 8)
#define RCC_APB1HENR_TIM23EN (1U << 0)
#define RCC_APB1HENR_TIM24EN (1U << 1)
#define RCC_APB2ENR_TIM1EN (1U << 0)
#define RCC_APB2ENR_TIM8EN (1U << 1)
#define RCC_APB2ENR_TIM15EN (1U << 2)
#define RCC_APB2ENR_TIM16EN (1U << 3)
#define RCC_APB2ENR_TIM17EN (1U << 4)

#define RCC_AHB4ENR_GPIOAEN (1U << 0)
#define RCC_AHB4ENR_GPIOBEN (1U << 1)
#define RCC_AHB4ENR_GPIOCEN (1U << 2)
#define RCC_AHB4ENR_GPIODEN (1U << 3)
#define RCC_AHB4ENR_GPIOEEN (1U << 4)
#define RCC_AHB4ENR_GPIOFEN (1U << 5)
#define RCC_AHB4ENR_GPIOGEN (1U << 6)
#define RCC_AHB4ENR_GPIOHEN (1U << 7)

#define RCC_APB4ENR_ADC12EN (1U << 5)
#define RCC_APB4ENR_ADC3EN (1U << 6)
#define RCC_APB2ENR_SPI1EN (1U << 12)
#define RCC_APB1LENR_SPI2EN (1U << 14)
#define RCC_APB1LENR_SPI3EN (1U << 15)
#define RCC_APB2ENR_SPI4EN (1U << 13)
#define RCC_APB2ENR_SPI5EN (1U << 20)
#define RCC_APB4ENR_SPI6EN (1U << 11)

#define __HAL_RCC_GPIOA_CLK_ENABLE() (RCC->AHB4ENR |= RCC_AHB4ENR_GPIOAEN)
#define __HAL_RCC_GPIOB_CLK_ENABLE() (RCC->AHB4ENR |= RCC_AHB4ENR_GPIOBEN)
#define __HAL_RCC_GPIOC_CLK_ENABLE() (RCC->AHB4ENR |= RCC_AHB4ENR_GPIOCEN)
#define __HAL_RCC_GPIOD_CLK_ENABLE() (RCC->AHB4ENR |= RCC_AHB4ENR_GPIODEN)
#define __HAL_RCC_GPIOE_CLK_ENABLE() (RCC->AHB4ENR |= RCC_AHB4ENR_GPIOEEN)
#define __HAL_RCC_GPIOF_CLK_ENABLE() (RCC->AHB4ENR |= RCC_AHB4ENR_GPIOFEN)
#define __HAL_RCC_GPIOG_CLK_ENABLE() (RCC->AHB4ENR |= RCC_AHB4ENR_GPIOGEN)
#define __HAL_RCC_GPIOH_CLK_ENABLE() (RCC->AHB4ENR |= RCC_AHB4ENR_GPIOHEN)
#define __HAL_RCC_ADC12_CLK_ENABLE() (RCC->APB4ENR |= RCC_APB4ENR_ADC12EN)
#define __HAL_RCC_ADC3_CLK_ENABLE() (RCC->APB4ENR |= RCC_APB4ENR_ADC3EN)
#define __HAL_RCC_SPI1_CLK_ENABLE() (RCC->APB2ENR |= RCC_APB2ENR_SPI1EN)
#define __HAL_RCC_SPI2_CLK_ENABLE() (RCC->APB1LENR |= RCC_APB1LENR_SPI2EN)
#define __HAL_RCC_SPI3_CLK_ENABLE() (RCC->APB1LENR |= RCC_APB1LENR_SPI3EN)
#define __HAL_RCC_SPI4_CLK_ENABLE() (RCC->APB2ENR |= RCC_APB2ENR_SPI4EN)
#define __HAL_RCC_SPI5_CLK_ENABLE() (RCC->APB2ENR |= RCC_APB2ENR_SPI5EN)
#define __HAL_RCC_SPI6_CLK_ENABLE() (RCC->APB4ENR |= RCC_APB4ENR_SPI6EN)
#define __HAL_RCC_DMA1_CLK_ENABLE() ((void)0U)
#define __HAL_RCC_DMA2_CLK_ENABLE() ((void)0U)

typedef struct __DMA_Stream_TypeDef {
    volatile uint32_t CR;
    volatile uint32_t NDTR;
    volatile uint32_t PAR;
    volatile uint32_t M0AR;
    volatile uint32_t M1AR;
    volatile uint32_t FCR;
} DMA_Stream_TypeDef;

extern DMA_Stream_TypeDef* DMA1_Stream0;
extern DMA_Stream_TypeDef* DMA1_Stream1;
extern DMA_Stream_TypeDef* DMA1_Stream2;
extern DMA_Stream_TypeDef* DMA1_Stream3;
extern DMA_Stream_TypeDef* DMA1_Stream4;
extern DMA_Stream_TypeDef* DMA1_Stream5;
extern DMA_Stream_TypeDef* DMA1_Stream6;
extern DMA_Stream_TypeDef* DMA1_Stream7;
extern DMA_Stream_TypeDef* DMA2_Stream0;
extern DMA_Stream_TypeDef* DMA2_Stream1;
extern DMA_Stream_TypeDef* DMA2_Stream2;
extern DMA_Stream_TypeDef* DMA2_Stream3;
extern DMA_Stream_TypeDef* DMA2_Stream4;
extern DMA_Stream_TypeDef* DMA2_Stream5;
extern DMA_Stream_TypeDef* DMA2_Stream6;
extern DMA_Stream_TypeDef* DMA2_Stream7;

typedef struct {
    volatile uint32_t MODER;
    volatile uint32_t OTYPER;
    volatile uint32_t OSPEEDR;
    volatile uint32_t PUPDR;
    volatile uint32_t IDR;
    volatile uint32_t ODR;
    volatile uint32_t BSRR;
    volatile uint32_t LCKR;
    volatile uint32_t AFR[2];
} GPIO_TypeDef;
extern GPIO_TypeDef* GPIOA;
extern GPIO_TypeDef* GPIOB;
extern GPIO_TypeDef* GPIOC;
extern GPIO_TypeDef* GPIOD;
extern GPIO_TypeDef* GPIOE;
extern GPIO_TypeDef* GPIOF;
extern GPIO_TypeDef* GPIOG;
extern GPIO_TypeDef* GPIOH;

typedef enum { GPIO_PIN_RESET = 0U, GPIO_PIN_SET } GPIO_PinState;

typedef struct {
    uint32_t Pin;
    uint32_t Mode;
    uint32_t Pull;
    uint32_t Speed;
    uint32_t Alternate;
} GPIO_InitTypeDef;

#define GPIO_PIN_0 (1U << 0)
#define GPIO_PIN_1 (1U << 1)
#define GPIO_PIN_2 (1U << 2)
#define GPIO_PIN_3 (1U << 3)
#define GPIO_PIN_4 (1U << 4)
#define GPIO_PIN_5 (1U << 5)
#define GPIO_PIN_6 (1U << 6)
#define GPIO_PIN_7 (1U << 7)
#define GPIO_PIN_8 (1U << 8)
#define GPIO_PIN_9 (1U << 9)
#define GPIO_PIN_10 (1U << 10)
#define GPIO_PIN_11 (1U << 11)
#define GPIO_PIN_12 (1U << 12)
#define GPIO_PIN_13 (1U << 13)
#define GPIO_PIN_14 (1U << 14)
#define GPIO_PIN_15 (1U << 15)
#define GPIO_PIN_All 0xFFFFU

#define GPIO_MODE_INPUT 0x00000000U
#define GPIO_MODE_OUTPUT_PP 0x00000001U
#define GPIO_MODE_OUTPUT_OD 0x00000011U
#define GPIO_MODE_AF_PP 0x00000002U
#define GPIO_MODE_AF_OD 0x00000012U
#define GPIO_MODE_ANALOG 0x00000003U
#define GPIO_MODE_IT_RISING 0x10110000U
#define GPIO_MODE_IT_FALLING 0x10210000U
#define GPIO_MODE_IT_RISING_FALLING 0x10310000U

#define GPIO_NOPULL 0x00000000U
#define GPIO_PULLUP 0x00000001U
#define GPIO_PULLDOWN 0x00000002U

#define GPIO_SPEED_FREQ_LOW 0x00000000U
#define GPIO_SPEED_FREQ_MEDIUM 0x00000001U
#define GPIO_SPEED_FREQ_HIGH 0x00000002U
#define GPIO_SPEED_FREQ_VERY_HIGH 0x00000003U

typedef struct {
    volatile uint32_t CR;
    volatile uint32_t CFGR;
    volatile uint32_t SR;
    volatile uint32_t DR;
    volatile uint32_t SMPR1;
    volatile uint32_t SMPR2;
    volatile uint32_t SQR1;
    volatile uint32_t SQR2;
    volatile uint32_t SQR3;
    volatile uint32_t SQR4;
} ADC_TypeDef;
extern ADC_TypeDef* ADC1;
extern ADC_TypeDef* ADC2;
extern ADC_TypeDef* ADC3;

typedef enum {
    HAL_OK = 0x00U,
    HAL_ERROR = 0x01U,
    HAL_BUSY = 0x02U,
    HAL_TIMEOUT = 0x03U
} HAL_StatusTypeDef;

typedef enum { HAL_UNLOCKED = 0x00U, HAL_LOCKED = 0x01U } HAL_LockTypeDef;

typedef struct {
    uint32_t ClockPrescaler;
    uint32_t Resolution;
    uint32_t ScanConvMode;
    uint32_t EOCSelection;
    uint32_t LowPowerAutoWait;
    uint32_t ContinuousConvMode;
    uint32_t NbrOfConversion;
    uint32_t DiscontinuousConvMode;
    uint32_t NbrOfDiscConversion;
    uint32_t ExternalTrigConv;
    uint32_t ExternalTrigConvEdge;
    uint32_t ConversionDataManagement;
    uint32_t Overrun;
    uint32_t LeftBitShift;
    uint32_t OversamplingMode;
    uint32_t DMAContinuousRequests;
    uint32_t SamplingMode;
} ADC_InitTypeDef;

typedef struct __ADC_HandleTypeDef {
    ADC_TypeDef* Instance;
    ADC_InitTypeDef Init;
    HAL_LockTypeDef Lock;
    volatile uint32_t State;
    volatile uint32_t ErrorCode;
} ADC_HandleTypeDef;

typedef struct {
    uint32_t Channel;
    uint32_t Rank;
    uint32_t SamplingTime;
    uint32_t SingleDiff;
    uint32_t OffsetNumber;
    uint32_t Offset;
    uint32_t OffsetSignedSaturation;
} ADC_ChannelConfTypeDef;

#define ADC_VER_V5_V90
#define ADC_RESOLUTION_16B 0x0U
#define ADC_RESOLUTION_14B 0x1U
#define ADC_RESOLUTION_12B 0x2U
#define ADC_RESOLUTION_10B 0x3U
#define ADC_RESOLUTION_8B 0x4U

#define ADC_SAMPLETIME_1CYCLE_5 0x0U
#define ADC_SAMPLETIME_2CYCLES_5 0x1U
#define ADC_SAMPLETIME_8CYCLES_5 0x2U
#define ADC_SAMPLETIME_16CYCLES_5 0x3U
#define ADC_SAMPLETIME_32CYCLES_5 0x4U
#define ADC_SAMPLETIME_64CYCLES_5 0x5U
#define ADC_SAMPLETIME_387CYCLES_5 0x6U
#define ADC_SAMPLETIME_810CYCLES_5 0x7U

#define ADC_CLOCK_ASYNC_DIV1 0x0U
#define ADC_CLOCK_ASYNC_DIV2 0x1U
#define ADC_CLOCK_ASYNC_DIV4 0x2U
#define ADC_CLOCK_ASYNC_DIV6 0x3U
#define ADC_CLOCK_ASYNC_DIV8 0x4U
#define ADC_CLOCK_ASYNC_DIV10 0x5U
#define ADC_CLOCK_ASYNC_DIV12 0x6U
#define ADC_CLOCK_ASYNC_DIV16 0x7U
#define ADC_CLOCK_ASYNC_DIV32 0x8U
#define ADC_CLOCK_ASYNC_DIV64 0x9U
#define ADC_CLOCK_ASYNC_DIV128 0xAU
#define ADC_CLOCK_ASYNC_DIV256 0xBU

#define ADC_CHANNEL_0 0U
#define ADC_CHANNEL_1 1U
#define ADC_CHANNEL_2 2U
#define ADC_CHANNEL_3 3U
#define ADC_CHANNEL_4 4U
#define ADC_CHANNEL_5 5U
#define ADC_CHANNEL_6 6U
#define ADC_CHANNEL_7 7U
#define ADC_CHANNEL_8 8U
#define ADC_CHANNEL_9 9U
#define ADC_CHANNEL_10 10U
#define ADC_CHANNEL_11 11U
#define ADC_CHANNEL_12 12U
#define ADC_CHANNEL_13 13U
#define ADC_CHANNEL_14 14U
#define ADC_CHANNEL_15 15U
#define ADC_CHANNEL_16 16U
#define ADC_CHANNEL_17 17U
#define ADC_CHANNEL_18 18U
#define ADC_CHANNEL_19 19U
#define ADC_CHANNEL_VREFINT 30U
#define ADC_CHANNEL_TEMPSENSOR 31U
#define ADC_CHANNEL_VBAT 32U

#define ADC_SCAN_DISABLE 0U
#define ADC_EOC_SINGLE_CONV 0U
#define ADC_SOFTWARE_START 0U
#define ADC_EXTERNALTRIGCONVEDGE_NONE 0U
#define ADC_CONVERSIONDATA_DR 0U
#define ADC_SAMPLING_MODE_NORMAL 0U
#define ADC_OVR_DATA_PRESERVED 0U
#define ADC_LEFTBITSHIFT_NONE 0U
#define ADC_REGULAR_RANK_1 1U
#define ADC_SINGLE_ENDED 0U
#define ADC_OFFSET_NONE 0U

#define HAL_ADC_ERROR_NONE 0U
#define HAL_ADC_STATE_RESET 0x00000000U
#define HAL_ADC_STATE_READY 0x00000001U
#define HAL_ADC_STATE_REG_BUSY 0x00000100U
#define HAL_ADC_STATE_REG_EOC 0x00000200U
#define HAL_ADC_STATE_TIMEOUT 0x00000400U

typedef struct {
    uint32_t Request;
    uint32_t Direction;
    uint32_t PeriphInc;
    uint32_t MemInc;
    uint32_t PeriphDataAlignment;
    uint32_t MemDataAlignment;
    uint32_t Mode;
    uint32_t Priority;
    uint32_t FIFOMode;
    uint32_t FIFOThreshold;
    uint32_t MemBurst;
    uint32_t PeriphBurst;
} DMA_InitTypeDef;

typedef struct __DMA_HandleTypeDef {
    DMA_Stream_TypeDef* Instance;
    DMA_InitTypeDef Init;
    void* Parent;
} DMA_HandleTypeDef;

#define DMA_REQUEST_MEM2MEM 0x000U
#define DMA_REQUEST_ADC1 0x001U
#define DMA_REQUEST_ADC2 0x002U
#define DMA_REQUEST_ADC3 0x003U
#define DMA_REQUEST_I2C1_RX 0x004U
#define DMA_REQUEST_I2C1_TX 0x005U
#define DMA_REQUEST_I2C2_RX 0x006U
#define DMA_REQUEST_I2C2_TX 0x007U
#define DMA_REQUEST_I2C3_RX 0x008U
#define DMA_REQUEST_I2C3_TX 0x009U
#define DMA_REQUEST_I2C5_RX 0x00AU
#define DMA_REQUEST_I2C5_TX 0x00BU
#define DMA_REQUEST_SPI1_RX 0x00CU
#define DMA_REQUEST_SPI1_TX 0x00DU
#define DMA_REQUEST_SPI2_RX 0x00EU
#define DMA_REQUEST_SPI2_TX 0x00FU
#define DMA_REQUEST_SPI3_RX 0x010U
#define DMA_REQUEST_SPI3_TX 0x011U
#define DMA_REQUEST_SPI4_RX 0x012U
#define DMA_REQUEST_SPI4_TX 0x013U
#define DMA_REQUEST_SPI5_RX 0x014U
#define DMA_REQUEST_SPI5_TX 0x015U
#define DMA_REQUEST_FMAC_WRITE 0x016U
#define DMA_REQUEST_FMAC_READ 0x017U

#define DMA_PERIPH_TO_MEMORY 0x000U
#define DMA_MEMORY_TO_PERIPH 0x001U
#define DMA_MEMORY_TO_MEMORY 0x002U

#define DMA_PINC_DISABLE 0x000U
#define DMA_PINC_ENABLE 0x001U
#define DMA_MINC_DISABLE 0x000U
#define DMA_MINC_ENABLE 0x001U

#define DMA_PDATAALIGN_BYTE 0x000U
#define DMA_PDATAALIGN_HALFWORD 0x001U
#define DMA_PDATAALIGN_WORD 0x002U
#define DMA_MDATAALIGN_BYTE 0x000U
#define DMA_MDATAALIGN_HALFWORD 0x001U
#define DMA_MDATAALIGN_WORD 0x002U

#define DMA_NORMAL 0x000U
#define DMA_CIRCULAR 0x001U
#define DMA_PRIORITY_LOW 0x000U
#define DMA_PRIORITY_HIGH 0x001U

#define DMA_FIFOMODE_DISABLE 0x000U
#define DMA_FIFOMODE_ENABLE 0x001U
#define DMA_FIFO_THRESHOLD_HALFFULL 0x000U
#define DMA_FIFO_THRESHOLD_FULL 0x001U

#define DMA_MBURST_SINGLE 0x000U
#define DMA_PBURST_SINGLE 0x000U

typedef struct {
    volatile uint32_t CR1;
    volatile uint32_t CR2;
    volatile uint32_t CFG1;
    volatile uint32_t CFG2;
    volatile uint32_t IER;
    volatile uint32_t SR;
    volatile uint32_t IFCR;
    volatile uint32_t TXDR;
    volatile uint32_t RXDR;
    volatile uint32_t CRCPOLY;
    volatile uint32_t TXCRC;
    volatile uint32_t RXCRC;
    volatile uint32_t UDRDR;
} SPI_TypeDef;

extern SPI_TypeDef* SPI1;
extern SPI_TypeDef* SPI2;
extern SPI_TypeDef* SPI3;
extern SPI_TypeDef* SPI4;
extern SPI_TypeDef* SPI5;
extern SPI_TypeDef* SPI6;

#define SPI1_BASE (0x40013000UL)
#define SPI2_BASE (0x40003800UL)
#define SPI3_BASE (0x40003C00UL)
#define SPI4_BASE (0x40013400UL)
#define SPI5_BASE (0x40015000UL)
#define SPI6_BASE (0x58001400UL)

typedef struct {
    uint32_t Mode;
    uint32_t Direction;
    uint32_t DataSize;
    uint32_t CLKPolarity;
    uint32_t CLKPhase;
    uint32_t NSS;
    uint32_t BaudRatePrescaler;
    uint32_t FirstBit;
    uint32_t TIMode;
    uint32_t CRCCalculation;
    uint32_t CRCPolynomial;
    uint32_t CRCLength;
    uint32_t NSSPMode;
    uint32_t NSSPolarity;
    uint32_t FifoThreshold;
    uint32_t TxCRCInitializationPattern;
    uint32_t RxCRCInitializationPattern;
    uint32_t MasterSSIdleness;
    uint32_t MasterInterDataIdleness;
    uint32_t MasterReceiverAutoSusp;
    uint32_t MasterKeepIOState;
    uint32_t IOSwap;
} SPI_InitTypeDef;

typedef struct __SPI_HandleTypeDef {
    SPI_TypeDef* Instance;
    SPI_InitTypeDef Init;
    DMA_HandleTypeDef* hdmatx;
    DMA_HandleTypeDef* hdmarx;
    HAL_LockTypeDef Lock;
    volatile uint32_t State;
    volatile uint32_t ErrorCode;
} SPI_HandleTypeDef;

#define SPI_MODE_SLAVE 0x00000000U
#define SPI_MODE_MASTER 0x00000001U

#define SPI_DIRECTION_2LINES 0x00000000U
#define SPI_DIRECTION_2LINES_RXONLY 0x00000001U
#define SPI_DIRECTION_1LINE 0x00000002U
#define SPI_DIRECTION_2LINES_TXONLY 0x00000003U

#define SPI_POLARITY_LOW 0x00000000U
#define SPI_POLARITY_HIGH 0x00000001U
#define SPI_PHASE_1EDGE 0x00000000U
#define SPI_PHASE_2EDGE 0x00000001U

#define SPI_NSS_SOFT 0x00000000U
#define SPI_NSS_HARD_INPUT 0x00000001U
#define SPI_NSS_HARD_OUTPUT 0x00000002U

#define SPI_BAUDRATEPRESCALER_2 0x00000000U
#define SPI_BAUDRATEPRESCALER_4 0x00000001U
#define SPI_BAUDRATEPRESCALER_8 0x00000002U
#define SPI_BAUDRATEPRESCALER_16 0x00000003U
#define SPI_BAUDRATEPRESCALER_32 0x00000004U
#define SPI_BAUDRATEPRESCALER_64 0x00000005U
#define SPI_BAUDRATEPRESCALER_128 0x00000006U
#define SPI_BAUDRATEPRESCALER_256 0x00000007U

#define SPI_FIRSTBIT_MSB 0x00000000U
#define SPI_FIRSTBIT_LSB 0x00000001U

#define SPI_TIMODE_DISABLE 0x00000000U
#define SPI_TIMODE_ENABLE 0x00000001U

#define SPI_CRCCALCULATION_DISABLE 0x00000000U
#define SPI_CRCCALCULATION_ENABLE 0x00000001U

#define SPI_NSS_PULSE_DISABLE 0x00000000U
#define SPI_NSS_PULSE_ENABLE 0x00000001U

#define SPI_NSS_POLARITY_LOW 0x00000000U
#define SPI_NSS_POLARITY_HIGH 0x00000001U

#define SPI_MASTER_KEEP_IO_STATE_DISABLE 0x00000000U
#define SPI_MASTER_KEEP_IO_STATE_ENABLE 0x00000001U
#define SPI_MASTER_RX_AUTOSUSP_DISABLE 0x00000000U
#define SPI_MASTER_RX_AUTOSUSP_ENABLE 0x00000001U
#define SPI_IO_SWAP_DISABLE 0x00000000U
#define SPI_IO_SWAP_ENABLE 0x00000001U

#define SPI_CRC_LENGTH_DATASIZE 0x00000000U
#define SPI_CRC_INITIALIZATION_ALL_ZERO_PATTERN 0x00000000U
#define SPI_CR1_SSI (1U << 8)

#define HAL_SPI_STATE_RESET 0x00000000U
#define HAL_SPI_STATE_READY 0x00000001U
#define HAL_SPI_STATE_BUSY 0x00000002U
#define HAL_SPI_ERROR_NONE 0x00000000U

typedef struct {
    uint32_t PeriphClockSelection;
    uint32_t Spi123ClockSelection;
    uint32_t Spi45ClockSelection;
    uint32_t Spi6ClockSelection;
} RCC_PeriphCLKInitTypeDef;

#define RCC_PERIPHCLK_SPI1 0x00000001U
#define RCC_PERIPHCLK_SPI2 0x00000002U
#define RCC_PERIPHCLK_SPI3 0x00000004U
#define RCC_PERIPHCLK_SPI4 0x00000008U
#define RCC_PERIPHCLK_SPI5 0x00000010U
#define RCC_PERIPHCLK_SPI6 0x00000020U
#define RCC_PERIPHCLK_SPI123 0x00000100U
#define RCC_PERIPHCLK_SPI45 0x00000200U

#define RCC_SPI123CLKSOURCE_PLL 0x00000001U
#define RCC_SPI45CLKSOURCE_PLL2 0x00000002U
#define RCC_SPI6CLKSOURCE_PLL2 0x00000002U

typedef struct TIM_TypeDef TIM_TypeDef;
extern TIM_TypeDef* TIM1_BASE;
extern TIM_TypeDef* TIM2_BASE;
extern TIM_TypeDef* TIM3_BASE;
extern TIM_TypeDef* TIM4_BASE;
extern TIM_TypeDef* TIM5_BASE;
extern TIM_TypeDef* TIM6_BASE;
extern TIM_TypeDef* TIM7_BASE;
extern TIM_TypeDef* TIM8_BASE;
extern TIM_TypeDef* TIM12_BASE;
extern TIM_TypeDef* TIM13_BASE;
extern TIM_TypeDef* TIM14_BASE;
extern TIM_TypeDef* TIM15_BASE;
extern TIM_TypeDef* TIM16_BASE;
extern TIM_TypeDef* TIM17_BASE;
extern TIM_TypeDef* TIM23_BASE;
extern TIM_TypeDef* TIM24_BASE;

#define TIM1 TIM1_BASE
#define TIM2 TIM2_BASE
#define TIM3 TIM3_BASE
#define TIM4 TIM4_BASE
#define TIM5 TIM5_BASE
#define TIM6 TIM6_BASE
#define TIM7 TIM7_BASE
#define TIM8 TIM8_BASE
#define TIM12 TIM12_BASE
#define TIM13 TIM13_BASE
#define TIM14 TIM14_BASE
#define TIM15 TIM15_BASE
#define TIM16 TIM16_BASE
#define TIM17 TIM17_BASE
#define TIM23 TIM23_BASE
#define TIM24 TIM24_BASE

typedef struct {
    uint32_t Prescaler;
    uint32_t CounterMode;
    uint32_t Period;
    uint32_t ClockDivision;
    uint32_t RepetitionCounter;
    uint32_t AutoReloadPreload;
} TIM_Base_InitTypeDef;

typedef enum {
    HAL_TIM_STATE_RESET = 0x00U,
    HAL_TIM_STATE_READY = 0x01U,
    HAL_TIM_STATE_BUSY = 0x02U
} HAL_TIM_StateTypeDef;

typedef enum {
    HAL_TIM_CHANNEL_STATE_RESET = 0x00U,
    HAL_TIM_CHANNEL_STATE_READY = 0x01U,
    HAL_TIM_CHANNEL_STATE_BUSY = 0x02U
} HAL_TIM_ChannelStateTypeDef;

typedef struct __TIM_HandleTypeDef {
    TIM_TypeDef* Instance;
    TIM_Base_InitTypeDef Init;
    HAL_TIM_ChannelStateTypeDef ChannelState[6];
    HAL_TIM_ChannelStateTypeDef ChannelNState[6];
    uint32_t DMABurstState;
    HAL_LockTypeDef Lock;
    HAL_TIM_StateTypeDef State;
} TIM_HandleTypeDef;

typedef struct {
    uint32_t OCMode;
    uint32_t Pulse;
    uint32_t OCPolarity;
    uint32_t OCNPolarity;
    uint32_t OCFastMode;
    uint32_t OCIdleState;
    uint32_t OCNIdleState;
} TIM_OC_InitTypeDef;

typedef struct {
    uint32_t EncoderMode;
    uint32_t IC1Polarity;
    uint32_t IC1Selection;
    uint32_t IC1Prescaler;
    uint32_t IC1Filter;
    uint32_t IC2Polarity;
    uint32_t IC2Selection;
    uint32_t IC2Prescaler;
    uint32_t IC2Filter;
} TIM_Encoder_InitTypeDef;

typedef struct {
    uint32_t MasterOutputTrigger;
    uint32_t MasterOutputTrigger2;
    uint32_t MasterSlaveMode;
} TIM_MasterConfigTypeDef;

typedef struct {
    uint32_t OffStateRunMode;
    uint32_t OffStateIDLEMode;
    uint32_t LockLevel;
    uint32_t DeadTime;
    uint32_t BreakState;
    uint32_t BreakPolarity;
    uint32_t BreakFilter;
    uint32_t BreakAFMode;
    uint32_t Break2State;
    uint32_t Break2Polarity;
    uint32_t Break2Filter;
    uint32_t Break2AFMode;
    uint32_t AutomaticOutput;
} TIM_BreakDeadTimeConfigTypeDef;

#define HAL_DMA_BURST_STATE_READY 0U

#define TIM_COUNTERMODE_UP 0U
#define TIM_CLOCKDIVISION_DIV1 0U
#define TIM_AUTORELOAD_PRELOAD_DISABLE 0U

#define TIM_OCMODE_PWM1 0x0060U
#define TIM_OCFAST_DISABLE 0U
#define TIM_OCIDLESTATE_RESET 0U
#define TIM_OCNIDLESTATE_RESET 0U
#define TIM_OCPOLARITY_HIGH 0U
#define TIM_OCNPOLARITY_HIGH 0U

#define TIM_ENCODERMODE_TI12 0U
#define TIM_ICPOLARITY_RISING 0U
#define TIM_ICSELECTION_DIRECTTI 0U
#define TIM_ICPSC_DIV1 0U

#define TIM_TRGO_RESET 0U
#define TIM_TRGO2_RESET 0U
#define TIM_MASTERSLAVEMODE_DISABLE 0U
#define TIM_BREAK_ENABLE 1U
#define TIM_LOCKLEVEL_OFF 0U

#define TIM_CHANNEL_1 0x00000000U
#define TIM_CHANNEL_2 0x00000004U
#define TIM_CHANNEL_3 0x00000008U
#define TIM_CHANNEL_4 0x0000000CU
#define TIM_CHANNEL_5 0x00000010U
#define TIM_CHANNEL_6 0x00000014U
#define TIM_CHANNEL_ALL 0x0000003CU

#define TIM_CR1_CEN (1U << 0)
#define TIM_CR1_UDIS (1U << 1)
#define TIM_CR1_OPM (1U << 3)
#define TIM_CR1_DIR (1U << 4)
#define TIM_CR1_CMS_0 (1U << 5)
#define TIM_CR1_CMS_1 (1U << 6)
#define TIM_CR1_CMS (TIM_CR1_CMS_0 | TIM_CR1_CMS_1)
#define TIM_CR1_CKD (3U << 8)

#define TIM_CR2_OIS1 (1U << 8)
#define TIM_CR2_OIS1N (1U << 9)
#define TIM_CR2_OIS2 (1U << 10)
#define TIM_CR2_OIS2N (1U << 11)
#define TIM_CR2_OIS3 (1U << 12)
#define TIM_CR2_OIS3N (1U << 13)
#define TIM_CR2_OIS4 (1U << 14)
#define TIM_CR2_OIS5 (1U << 16)
#define TIM_CR2_OIS6 (1U << 18)

#define TIM_SMCR_SMS (7U << 0)
#define TIM_SMCR_ECE (1U << 14)

#define TIM_DIER_UIE (1U << 0)
#define TIM_DIER_BIE (1U << 7)
#define TIM_SR_UIF (1U << 0)
#define TIM_SR_COMIF (1U << 5)
#define TIM_SR_TIF (1U << 6)
#define TIM_SR_BIF (1U << 7)

#define TIM_CCMR1_CC1S (3U << 0)
#define TIM_CCMR1_OC1PE (1U << 3)
#define TIM_CCMR1_OC1M (7U << 4)
#define TIM_CCMR1_CC2S (3U << 8)
#define TIM_CCMR1_OC2PE (1U << 11)
#define TIM_CCMR1_OC2M (7U << 12)

#define TIM_CCMR2_CC3S (3U << 0)
#define TIM_CCMR2_OC3PE (1U << 3)
#define TIM_CCMR2_OC3M (7U << 4)
#define TIM_CCMR2_CC4S (3U << 8)
#define TIM_CCMR2_OC4PE (1U << 11)
#define TIM_CCMR2_OC4M (7U << 12)

#define TIM_CCMR3_OC5PE (1U << 3)
#define TIM_CCMR3_OC5M (7U << 4)
#define TIM_CCMR3_OC6PE (1U << 11)
#define TIM_CCMR3_OC6M (7U << 12)

#define TIM_CCER_CC1E (1U << 0)
#define TIM_CCER_CC1P (1U << 1)
#define TIM_CCER_CC1NE (1U << 2)
#define TIM_CCER_CC1NP (1U << 3)
#define TIM_CCER_CC2E (1U << 4)
#define TIM_CCER_CC2P (1U << 5)
#define TIM_CCER_CC2NE (1U << 6)
#define TIM_CCER_CC2NP (1U << 7)
#define TIM_CCER_CC3E (1U << 8)
#define TIM_CCER_CC3P (1U << 9)
#define TIM_CCER_CC3NE (1U << 10)
#define TIM_CCER_CC3NP (1U << 11)
#define TIM_CCER_CC4E (1U << 12)
#define TIM_CCER_CC4P (1U << 13)
#define TIM_CCER_CC5E (1U << 16)
#define TIM_CCER_CC5P (1U << 17)
#define TIM_CCER_CC6E (1U << 20)
#define TIM_CCER_CC6P (1U << 21)
#define TIM_CCER_CCxE_MASK                                                                         \
    (TIM_CCER_CC1E | TIM_CCER_CC2E | TIM_CCER_CC3E | TIM_CCER_CC4E | TIM_CCER_CC5E | TIM_CCER_CC6E)
#define TIM_CCER_CCxNE_MASK (TIM_CCER_CC1NE | TIM_CCER_CC2NE | TIM_CCER_CC3NE)

#define TIM_BDTR_MOE (1U << 15)

#define LL_TIM_DIER_UIE TIM_DIER_UIE
#define LL_TIM_SR_UIF TIM_SR_UIF
#define LL_TIM_CLOCKDIVISION_DIV1 TIM_CLOCKDIVISION_DIV1
#define LL_TIM_EnableCounter(__TIMx) ((__TIMx)->CR1 |= TIM_CR1_CEN)
#define LL_TIM_DisableCounter(__TIMx) ((__TIMx)->CR1 &= ~TIM_CR1_CEN)

#define IS_TIM_SLAVEMODE_TRIGGER_ENABLED(__SMCR__) ((((__SMCR__) & TIM_SMCR_SMS) != 0U) ? 1U : 0U)

#define SYSCFG_PMCR_PC2SO (1U << 2)
#define SYSCFG_PMCR_PC3SO (1U << 3)

HAL_StatusTypeDef HAL_ADC_Init(ADC_HandleTypeDef* hadc);
HAL_StatusTypeDef HAL_ADC_DeInit(ADC_HandleTypeDef* hadc);
HAL_StatusTypeDef HAL_ADC_ConfigChannel(ADC_HandleTypeDef* hadc, ADC_ChannelConfTypeDef* sConfig);
HAL_StatusTypeDef HAL_ADC_Start(ADC_HandleTypeDef* hadc);
HAL_StatusTypeDef HAL_ADC_PollForConversion(ADC_HandleTypeDef* hadc, uint32_t Timeout);
uint32_t HAL_ADC_GetValue(const ADC_HandleTypeDef* hadc);
HAL_StatusTypeDef HAL_ADC_Stop(ADC_HandleTypeDef* hadc);
uint32_t HAL_ADC_GetState(const ADC_HandleTypeDef* hadc);
uint32_t HAL_ADC_GetError(const ADC_HandleTypeDef* hadc);

HAL_StatusTypeDef HAL_SPI_Init(SPI_HandleTypeDef* hspi);
HAL_StatusTypeDef HAL_SPI_Abort(SPI_HandleTypeDef* hspi);
HAL_StatusTypeDef
HAL_SPI_Transmit(SPI_HandleTypeDef* hspi, uint8_t* pData, uint16_t Size, uint32_t Timeout);
HAL_StatusTypeDef
HAL_SPI_Receive(SPI_HandleTypeDef* hspi, uint8_t* pData, uint16_t Size, uint32_t Timeout);
HAL_StatusTypeDef HAL_SPI_TransmitReceive(
    SPI_HandleTypeDef* hspi,
    uint8_t* pTxData,
    uint8_t* pRxData,
    uint16_t Size,
    uint32_t Timeout
);
HAL_StatusTypeDef HAL_SPI_Transmit_DMA(SPI_HandleTypeDef* hspi, uint8_t* pData, uint16_t Size);
HAL_StatusTypeDef HAL_SPI_Receive_DMA(SPI_HandleTypeDef* hspi, uint8_t* pData, uint16_t Size);
HAL_StatusTypeDef HAL_SPI_TransmitReceive_DMA(
    SPI_HandleTypeDef* hspi,
    uint8_t* pTxData,
    uint8_t* pRxData,
    uint16_t Size
);
void HAL_SPI_IRQHandler(SPI_HandleTypeDef* hspi);

void HAL_SYSCFG_AnalogSwitchConfig(uint32_t SYSCFG_AnalogSwitch, uint32_t SYSCFG_SwitchState);

void NVIC_EnableIRQ(IRQn_Type IRQn);
void NVIC_DisableIRQ(IRQn_Type IRQn);

HAL_StatusTypeDef MockedHAL_DMA_Init_Impl(DMA_HandleTypeDef* hdma);
HAL_StatusTypeDef MockedHAL_DMA_Start_IT_Impl(
    DMA_HandleTypeDef* hdma,
    uint32_t SrcAddress,
    uint32_t DstAddress,
    uint32_t DataLength
);
void MockedHAL_DMA_IRQHandler_Impl(DMA_HandleTypeDef* hdma);

static inline uint32_t HAL_RCC_GetPCLK1Freq(void) { return SystemCoreClock; }
static inline uint32_t HAL_RCC_GetPCLK2Freq(void) { return SystemCoreClock; }
static inline void HAL_NVIC_SetPriority(IRQn_Type IRQn, uint32_t priority, uint32_t subpriority) {
    (void)IRQn;
    (void)priority;
    (void)subpriority;
}
static inline void HAL_NVIC_EnableIRQ(IRQn_Type IRQn) { NVIC_EnableIRQ(IRQn); }
static inline void HAL_NVIC_DisableIRQ(IRQn_Type IRQn) { NVIC_DisableIRQ(IRQn); }
static inline HAL_StatusTypeDef HAL_RCCEx_PeriphCLKConfig(RCC_PeriphCLKInitTypeDef* PeriphClkInit) {
    (void)PeriphClkInit;
    return HAL_OK;
}
static inline uint32_t HAL_RCCEx_GetPeriphCLKFreq(uint32_t PeriphClk) {
    (void)PeriphClk;
    return SystemCoreClock;
}
static inline HAL_StatusTypeDef HAL_DMA_Init(DMA_HandleTypeDef* hdma) {
    return MockedHAL_DMA_Init_Impl(hdma);
}
static inline HAL_StatusTypeDef HAL_DMA_Start_IT(
    DMA_HandleTypeDef* hdma,
    uint32_t SrcAddress,
    uint32_t DstAddress,
    uint32_t DataLength
) {
    return MockedHAL_DMA_Start_IT_Impl(hdma, SrcAddress, DstAddress, DataLength);
}
static inline void HAL_DMA_IRQHandler(DMA_HandleTypeDef* hdma) {
    MockedHAL_DMA_IRQHandler_Impl(hdma);
}

static inline void HAL_GPIO_Init(GPIO_TypeDef* GPIOx, GPIO_InitTypeDef* GPIO_Init) {
    (void)GPIOx;
    (void)GPIO_Init;
}

static inline void
HAL_GPIO_WritePin(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, GPIO_PinState PinState) {
    if (PinState == GPIO_PIN_SET) {
        GPIOx->ODR |= GPIO_Pin;
    } else {
        GPIOx->ODR &= ~((uint32_t)GPIO_Pin);
    }
}

static inline void HAL_GPIO_TogglePin(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin) {
    GPIOx->ODR ^= GPIO_Pin;
}

static inline GPIO_PinState HAL_GPIO_ReadPin(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin) {
    return (GPIOx->ODR & GPIO_Pin) ? GPIO_PIN_SET : GPIO_PIN_RESET;
}

static inline HAL_StatusTypeDef
HAL_TIM_Encoder_Init(TIM_HandleTypeDef* htim, const TIM_Encoder_InitTypeDef* sConfig) {
    (void)sConfig;
    if (htim == NULL) {
        return HAL_ERROR;
    }
    htim->State = HAL_TIM_STATE_READY;
    return HAL_OK;
}

static inline HAL_StatusTypeDef HAL_TIMEx_MasterConfigSynchronization(
    TIM_HandleTypeDef* htim,
    const TIM_MasterConfigTypeDef* sMasterConfig
) {
    (void)sMasterConfig;
    if (htim == NULL) {
        return HAL_ERROR;
    }
    return HAL_OK;
}

static inline HAL_TIM_StateTypeDef HAL_TIM_Encoder_GetState(const TIM_HandleTypeDef* htim) {
    if (htim == NULL) {
        return HAL_TIM_STATE_RESET;
    }
    return htim->State;
}

static inline HAL_StatusTypeDef HAL_TIM_Encoder_Start(TIM_HandleTypeDef* htim, uint32_t Channel) {
    (void)Channel;
    if (htim == NULL) {
        return HAL_ERROR;
    }
    htim->State = HAL_TIM_STATE_BUSY;
    return HAL_OK;
}

static inline HAL_StatusTypeDef HAL_TIM_Encoder_Stop(TIM_HandleTypeDef* htim, uint32_t Channel) {
    (void)Channel;
    if (htim == NULL) {
        return HAL_ERROR;
    }
    htim->State = HAL_TIM_STATE_READY;
    return HAL_OK;
}

static inline HAL_StatusTypeDef HAL_TIMEx_ConfigBreakDeadTime(
    TIM_HandleTypeDef* htim,
    const TIM_BreakDeadTimeConfigTypeDef* sBreakDeadTimeConfig
) {
    (void)htim;
    (void)sBreakDeadTimeConfig;
    return HAL_OK;
}

#ifdef __cplusplus
}
#endif
