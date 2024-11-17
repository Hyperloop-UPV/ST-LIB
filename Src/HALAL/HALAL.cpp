/*
 * HALAL.cpp
 *
 *  Created on: 5 ene. 2023
 *      Author: aleja
 */

#include "HALAL/HALAL.hpp"

#ifndef SIM_ON
void HALAL::start(IPV4 ip, IPV4 subnet_mask, IPV4 gateway,
                  UART::Peripheral& printf_peripheral) {
#if !defined STLIB_ETH
#else
    Ethernet::inscribe();
#endif
    MPUManager::start();
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
    DMA::start();
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

#ifdef HAL_ADC_MODULE_ENABLED
    ADC::start();
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

#if !defined STLIB_ETH
#else
    Ethernet::start(ip, subnet_mask, gateway);
#endif

#ifdef HAL_TIM_MODULE_ENABLED
    Encoder::start();
    Global_RTC::start_rtc();
    SNTP::sntp_update();
    TimerPeripheral::start();
    Time::start();
#endif

#ifdef HAL_EXTI_MODULE_ENABLED
    ExternalInterrupt::start();
#endif
}
#else
// Simulator start
void HALAL::start(IPV4 ip, IPV4 subnet_mask, IPV4 gateway, UART::Peripheral& printf_peripheral) {
    SharedMemory::start();
    Ethernet::inscribe();
    Pin::start();
    ADC::start();
    SPI::start();
    FDCAN::start();
    Ethernet::start(ip, subnet_mask, gateway);
    Encoder::start();
    Global_RTC::start_rtc();
    Time::start();
    ExternalInterrupt::start();
}
#endif
