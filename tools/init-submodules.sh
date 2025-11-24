#!/usr/bin/env bash
set -euo pipefail

cd "$(git rev-parse --show-toplevel)"

git submodule update --init
cd STM32CubeH7
git submodule update --init Drivers/STM32H7xx_HAL_Driver Drivers/CMSIS/Device/ST/STM32H7xx Drivers/BSP/Components/lan8742
