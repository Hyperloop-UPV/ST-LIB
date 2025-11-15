# ST-LIB

Library to simplify programming an STM32 H7 MCU taking advantage of C++ objects.

The library is divided into three levels:

* HALAL (HAL Abstraction Layer): takes the STM32 HAL functions and exposes an object oriented interface
* STLIB-LOW: provides low level utilities on top of the HALAL and a couple of fundamental abstractions.
* STLIB-HIGH: provides high level programming utilities to validate sensor values and implement control loops.

## Container Setup
To use it you must install [Dev Containers extension on VSCode](https://marketplace.visualstudio.com/items?itemName=ms-vscode-remote.remote-containers) and [Docker](https://www.docker.com). Be careful,use the instructions related to your OS, as docker doesn't work the same way in all OS's.
Then, when you open this folder in VSCode, you will have the ability to reopen it inside the container. Don't worry, the first time you do it will take some time.

## Submodules setup
The only needed submodules are:
- STM32CubeH7/Drivers/STM32H7xx_HAL_Driver
- STM32CubeH7/Drivers/CMSIS/Device/ST/STM32H7xx
- STM32CubeH7/Drivers/BSP/Components/lan8742

```sh
git submodule init
git submodule update STM32CubeH7/Drivers/STM32H7xx_HAL_Driver STM32CubeH7/Drivers/CMSIS STM32CubeH7/Device/ST/STM32H7xx Drivers/BSP/Components/lan8742
```