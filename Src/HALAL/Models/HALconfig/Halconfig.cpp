/*
 * Halconfig.cpp
 *
 *  Created on: 5 ene. 2023
 *      Author: aleja
 */

#include "HALAL/Models/HALconfig/HALconfig.hpp"

void HALconfig::system_clock() {
	RCC_OscInitTypeDef RCC_OscInitStruct = {0};
	RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

	HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE0);

	while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

	__HAL_RCC_PLL_PLLSOURCE_CONFIG(RCC_PLLSOURCE_HSE);

	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.LSIState = RCC_LSI_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;

#ifdef NUCLEO
	RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
	RCC_OscInitStruct.PLL.PLLM = 4;
	RCC_OscInitStruct.PLL.PLLN = 275;
	RCC_OscInitStruct.PLL.PLLP = 1;
	RCC_OscInitStruct.PLL.PLLQ = 4;
	RCC_OscInitStruct.PLL.PLLR = 2;
	RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_1;
	RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
	RCC_OscInitStruct.PLL.PLLFRACN = 0;

#else
#ifdef BOARD
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.PLL.PLLM = 5;
	RCC_OscInitStruct.PLL.PLLN = 110;
	RCC_OscInitStruct.PLL.PLLP = 1;
	RCC_OscInitStruct.PLL.PLLQ = 4;
	RCC_OscInitStruct.PLL.PLLR = 2;
	RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_2;
	RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
	RCC_OscInitStruct.PLL.PLLFRACN = 0;
#else
static_assert(false, "No TARGET is choosen. Choose NUCLEO or BOARD");
#endif
#endif

	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
		// ErrorHandler("The RCC Osc config did not start correctly");
	}

	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
							  |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
							  |RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
	RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
	RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;
	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK) {
		// ErrorHandler("The RCC clock config did not start correctly");
	}
}




void HALconfig::peripheral_clock() {
	RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

#ifdef NUCLEO
	PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_ADC|RCC_PERIPHCLK_FDCAN;
	PeriphClkInitStruct.PLL2.PLL2M = 8;
	PeriphClkInitStruct.PLL2.PLL2N = 200;
	PeriphClkInitStruct.PLL2.PLL2P = 2;
	PeriphClkInitStruct.PLL2.PLL2Q = 10;
	PeriphClkInitStruct.PLL2.PLL2R = 2;
	PeriphClkInitStruct.PLL2.PLL2RGE = RCC_PLL2VCIRANGE_0;
	PeriphClkInitStruct.PLL2.PLL2VCOSEL = RCC_PLL2VCOMEDIUM;
	PeriphClkInitStruct.PLL2.PLL2FRACN = 0;
	PeriphClkInitStruct.PLL3.PLL3M = 8;
	PeriphClkInitStruct.PLL3.PLL3N = 192;
	PeriphClkInitStruct.PLL3.PLL3P = 2;
	PeriphClkInitStruct.PLL3.PLL3Q = 2;
	PeriphClkInitStruct.PLL3.PLL3R = 2;
	PeriphClkInitStruct.PLL3.PLL3RGE = RCC_PLL3VCIRANGE_0;
	PeriphClkInitStruct.PLL3.PLL3VCOSEL = RCC_PLL3VCOMEDIUM;
	PeriphClkInitStruct.PLL3.PLL3FRACN = 0;
	PeriphClkInitStruct.FdcanClockSelection = RCC_FDCANCLKSOURCE_PLL2;
	PeriphClkInitStruct.AdcClockSelection = RCC_ADCCLKSOURCE_PLL3;
#else
#ifdef BOARD
	PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_ADC|RCC_PERIPHCLK_FDCAN;
	PeriphClkInitStruct.PLL2.PLL2M = 25;
	PeriphClkInitStruct.PLL2.PLL2N = 200;
	PeriphClkInitStruct.PLL2.PLL2P = 2;
	PeriphClkInitStruct.PLL2.PLL2Q = 10;
	PeriphClkInitStruct.PLL2.PLL2R = 2;
	PeriphClkInitStruct.PLL2.PLL2RGE = RCC_PLL2VCIRANGE_0;
	PeriphClkInitStruct.PLL2.PLL2VCOSEL = RCC_PLL2VCOMEDIUM;
	PeriphClkInitStruct.PLL2.PLL2FRACN = 0;
	PeriphClkInitStruct.PLL3.PLL3M = 25;
	PeriphClkInitStruct.PLL3.PLL3N = 192;
	PeriphClkInitStruct.PLL3.PLL3P = 2;
	PeriphClkInitStruct.PLL3.PLL3Q = 2;
	PeriphClkInitStruct.PLL3.PLL3R = 2;
	PeriphClkInitStruct.PLL3.PLL3RGE = RCC_PLL3VCIRANGE_0;
	PeriphClkInitStruct.PLL3.PLL3VCOSEL = RCC_PLL3VCOMEDIUM;
	PeriphClkInitStruct.PLL3.PLL3FRACN = 0;
	PeriphClkInitStruct.FdcanClockSelection = RCC_FDCANCLKSOURCE_PLL2;
	PeriphClkInitStruct.AdcClockSelection = RCC_ADCCLKSOURCE_PLL3;

#else
	static_assert(false, "No TARGET is choosen. Choose NUCLEO or BOARD");
#endif
#endif

	if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK) {
		// ErrorHandler("The RCCEx peripheral clock did not start correctly");
	}
}
