#!/usr/bin/env bash
set -euo pipefail

# This script is supposed to be under ${ST-LIB_PATH}/tools/init-submodules.sh

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
STLIB_DIR="$(cd -- "${SCRIPT_DIR}/.." && pwd)"

cd "$STLIB_DIR"
git submodule update --init
cd "$STLIB_DIR/STM32CubeH7"
git submodule update --init \
  Drivers/STM32H7xx_HAL_Driver \
  Drivers/CMSIS/Device/ST/STM32H7xx \
  Drivers/BSP/Components/lan8742
