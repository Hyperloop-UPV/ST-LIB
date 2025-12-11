#ifndef __HARD_FAULT_TRACE

#define __HARD_FAULT_TRACE
#include "stm32h7xx_ll_gpio.h"
#include "stm32h7xx_ll_bus.h"
#include "stm32h7xx_ll_tim.h"
#define METADATA_FLASH_ADDR (0x080DFD00) //Metadata pool flash address
#define HF_FLASH_ADDR (0x080C0000U) //Hard_fault_flash address
#define HF_FLAG_VALUE (0xFF00FF00U) //Flag to know if already is written information in the flash
#define METADATA_FLASH_SIZE (0X100U)
#define HARD_FAULT_FLASH_SIZE (0X200U)
typedef struct __attribute__((packed)) ContextStateFrame {
  uint32_t r0;
  uint32_t r1;
  uint32_t r2;
  uint32_t r3;
  uint32_t r12;
  uint32_t lr;
  uint32_t return_address;
  uint32_t xpsr;
} sContextStateFrame;


typedef struct __attribute__((packed)) HardFaultLog{
  uint32_t HF_flag;
  sContextStateFrame frame;
  union{
    uint32_t cfsr;
  struct CfSrFields{
      uint8_t MMFSR;
      uint8_t BFSR;
      uint16_t UFSR;
    }fields;
  }CfsrDecode;
  union{
    uint32_t Nothing_Valid;
    uint32_t MMAR_VALID;
    uint32_t BFAR_VALID;
  }fault_address;
}HardFaultLog; // 44 bytes this estructure
// static void LED_init(void);
// static void LED_Blink(uint32_t delay_ms);
// static uint8_t HF_FLASH(void);
#ifdef __cplusplus
extern "C" {
#endif
void Hard_fault_check(void);

#ifdef __cplusplus
}
#endif

#endif