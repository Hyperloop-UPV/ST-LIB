/*
 * HALAL.cpp
 *
 *  Created on: 5 ene. 2023
 *      Author: aleja
 */

#include "HALAL/HALAL.hpp"
#include "stm32h7xx_hal.h"
#include "stm32h7xx_hal_eth.h"

#ifndef STLIB_ETH
ETH_HandleTypeDef heth;
void HAL_ETH_IRQHandler(ETH_HandleTypeDef *heth_arg) { (void)heth_arg; }
#endif // STLIB_ETH
namespace HALAL {

static void common_start(UART::Peripheral &printf_peripheral) {
#ifdef HAL_IWDG_MODULE_ENABLED
  Watchdog::check_reset_flag();
#endif

  HAL_Init();
  HALconfig::system_clock();
  HALconfig::peripheral_clock();

#ifdef HAL_UART_MODULE_ENABLED
  UART::set_up_printf(printf_peripheral);
#endif

#ifdef HAL_GPIO_MODULE_ENABLED
  Pin::start();
#endif

#ifdef HAL_DMA_MODULE_ENABLED
  // DMA::start();
#endif

#ifdef HAL_MDMA_MODULE_ENABLED
    MDMA::start();
#endif

#ifdef HAL_FMAC_MODULE_ENABLED
  MultiplierAccelerator::start();
#endif

#ifdef HAL_CORDIC_MODULE_ENABLED
  CORDIC_HandleTypeDef hcordic;
  hcordic.Instance = CORDIC;
  if (HAL_CORDIC_Init(&hcordic) != HAL_OK) {
    ErrorHandler("Unable to init CORDIC");
  }
#endif

#ifdef HAL_I2C_MODULE_ENABLED
  I2C::start();
#endif

#ifdef HAL_SPI_MODULE_ENABLED
  SPI::start();
#endif

#ifdef HAL_UART_MODULE_ENABLED
  UART::start();
#endif

#ifdef HAL_FDCAN_MODULE_ENABLED
  FDCAN::start();
#endif

#ifdef HAL_TIM_MODULE_ENABLED
  // Encoder::start();
  Global_RTC::start_rtc();
  //TimerPeripheral::start();
  //Time::start();
#endif

#ifdef NDEBUG
#ifdef HAL_IWDG_MODULE_ENABLED
  Watchdog::start();
#endif
#endif
}

#ifdef STLIB_ETH

void start(MAC mac, IPV4 ip, IPV4 subnet_mask, IPV4 gateway,
           UART::Peripheral &printf_peripheral) {
  Ethernet::inscribe();

  common_start(printf_peripheral);

  Ethernet::start(mac, ip, subnet_mask, gateway);

#ifdef HAL_TIM_MODULE_ENABLED
  SNTP::sntp_update();
#endif
}

#else // !STLIB_ETH

void start(UART::Peripheral &printf_peripheral) {
  common_start(printf_peripheral);
}

#endif // STLIB_ETH

} // namespace HALAL
