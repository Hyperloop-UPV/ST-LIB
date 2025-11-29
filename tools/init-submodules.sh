#!/usr/bin/env bash
set -euo pipefail

# This script is supposed to be under ${ST-LIB_PATH}/tools/init-submodules.sh

cd "${0%/*}/.."
git submodule update --init
cd STM32CubeH7
git submodule update --init Drivers/STM32H7xx_HAL_Driver Drivers/CMSIS/Device/ST/STM32H7xx Drivers/BSP/Components/lan8742
