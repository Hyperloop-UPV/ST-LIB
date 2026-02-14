# ST-LIB Setup

## 1. Prerequisites

- CMake + Ninja (CMake >= 3.28 for `cmake --preset` / `ctest --preset` with this repo)
- Python 3
- C/C++ compiler toolchain

For MCU builds:

- `arm-none-eabi-*` toolchain

## 2. Initialize Submodules

```sh
./tools/init-submodules.sh
```

Manual equivalent:

```sh
git submodule update --init
cd STM32CubeH7
git submodule update --init \
  Drivers/STM32H7xx_HAL_Driver \
  Drivers/CMSIS/Device/ST/STM32H7xx \
  Drivers/BSP/Components/lan8742
```

## 3. Automatic Formatting

```sh
pip install pre-commit
./tools/install-git-hooks.sh
```

Manual validation:

```sh
pre-commit run --all-files
```
