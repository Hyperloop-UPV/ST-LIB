/*
******************************************************************************
**
**  File        : LinkerScript.ld (debug in RAM dedicated)
**
**  Author      : STM32CubeIDE
**
**  Abstract    : Linker script for STM32H7 series
**                320Kbytes RAM_EXEC and 240Kbytes RAM
**
**                Set heap size, stack size and stack location according
**                to application requirements.
**
**                Set memory bank area and size if external memory is used.
**
**  Target      : STMicroelectronics STM32
**
**  Distribution: The file is distributed as is, without any warranty
**                of any kind.
**
*****************************************************************************
** @attention
**
** Copyright (c) 2022 STMicroelectronics.
** All rights reserved.
**
** This software is licensed under terms that can be found in the LICENSE file
** in the root directory of this software component.
** If no LICENSE file comes with this software, it is provided AS-IS.
**
****************************************************************************
*/

/* Entry Point */
ENTRY(Reset_Handler)

/* Highest address of the user mode stack */
_estack = ORIGIN(DTCMRAM) + LENGTH(DTCMRAM);    /* end of RAM */
/* Generate a link error if heap and stack don't fit into RAM */
_Min_Heap_Size = 0x200 ;      /* required amount of heap  */
_Min_Stack_Size = 0x400 ; /* required amount of stack */

/* Specify the memory areas */
MEMORY
{
  RAM_EXEC (xrw)  : ORIGIN = 0x24000000, LENGTH = 320K
  DTCMRAM  (xrw)  : ORIGIN = 0x20000000, LENGTH = 128K
  ITCMRAM (xrw)   : ORIGIN = 0x00000000, LENGTH = 64K
  RAM_D2  (xrw)   : ORIGIN = 0x30000000, LENGTH = 32K
  RAM_D3  (xrw)   : ORIGIN = 0x38000000, LENGTH = 16K
}

/* Define output sections */
SECTIONS
{
  /* Export Memory Layout Information for MPU Configuration */
  PROVIDE(__itcm_base = ORIGIN(ITCMRAM));
  PROVIDE(__itcm_size = LENGTH(ITCMRAM));
  PROVIDE(__dtcm_base = ORIGIN(DTCMRAM));
  PROVIDE(__dtcm_size = LENGTH(DTCMRAM));
  PROVIDE(__flash_base = ORIGIN(RAM_EXEC)); /* Emulate Flash as RAM_EXEC start for RAM mode */
  PROVIDE(__flash_size = LENGTH(RAM_EXEC));
  PROVIDE(__ram_d1_base = ORIGIN(RAM_EXEC));
  PROVIDE(__ram_d1_size = LENGTH(RAM_EXEC));
  PROVIDE(__ram_d2_base = ORIGIN(RAM_D2));
  PROVIDE(__ram_d2_size = LENGTH(RAM_D2));
  PROVIDE(__ram_d3_base = ORIGIN(RAM_D3));
  PROVIDE(__ram_d3_size = LENGTH(RAM_D3));
  PROVIDE(__peripheral_base = 0x40000000);
  PROVIDE(__peripheral_size = 0x20000000);

  /* The startup code goes first into RAM_EXEC */
  .isr_vector :
  {
    . = ALIGN(4);
    KEEP(*(.isr_vector)) /* Startup code */
    . = ALIGN(4);
  } >RAM_EXEC

  /* The program code and other data goes into RAM_EXEC */
  .text :
  {
    . = ALIGN(4);
    *(.text)           /* .text sections (code) */
    *(.text*)          /* .text* sections (code) */
    *(.glue_7)         /* glue arm to thumb code */
    *(.glue_7t)        /* glue thumb to arm code */
    *(.eh_frame)

    /* No implementation nor use for them just yet
    *(.RamFunc)        /* .RamFunc sections */
    *(.RamFunc*)       /* .RamFunc* sections */
    */

    KEEP (*(.init))
    KEEP (*(.fini))

    . = ALIGN(4);
    _etext = .;        /* define a global symbols at end of code */
  } >RAM_EXEC

  /* Constant data goes into RAM_EXEC */
  .rodata :
  {
    . = ALIGN(4);
    *(.rodata)         /* .rodata sections (constants, strings, etc.) */
    *(.rodata*)        /* .rodata* sections (constants, strings, etc.) */
    . = ALIGN(4);
  } >RAM_EXEC

  .ARM.extab (READONLY): { *(.ARM.extab* .gnu.linkonce.armextab.*) } >RAM_EXEC
  .ARM (READONLY): {
    __exidx_start = .;
    *(.ARM.exidx*)
    __exidx_end = .;
  } >RAM_EXEC

  .preinit_array (READONLY):
  {
    PROVIDE_HIDDEN (__preinit_array_start = .);
    KEEP (*(.preinit_array*))
    PROVIDE_HIDDEN (__preinit_array_end = .);
  } >RAM_EXEC

  .init_array (READONLY):
  {
    PROVIDE_HIDDEN (__init_array_start = .);
    KEEP (*(SORT(.init_array.*)))
    KEEP (*(.init_array*))
    PROVIDE_HIDDEN (__init_array_end = .);
  } >RAM_EXEC

  .fini_array (READONLY):
  {
    PROVIDE_HIDDEN (__fini_array_start = .);
    KEEP (*(SORT(.fini_array.*)))
    KEEP (*(.fini_array*))
    PROVIDE_HIDDEN (__fini_array_end = .);
  } >RAM_EXEC

  .mpu_ram_d1_nc :
  {
    . = ALIGN(32);
    PROVIDE(__mpu_d1_nc_start = .);
    *(.mpu_ram_d1_nc)
    . = ALIGN(32);
    *(.Rx_PoolSection)
  } >RAM_EXEC

  /* CALCULATE PADDING FOR MPU SUBREGIONS (D1) */
  _d1_size = SIZEOF(.mpu_ram_d1_nc);
  /* Find next power of 2 */
  _d1_p2 = (_d1_size <= 32) ? 32 : (_d1_size <= 64) ? 64 : (_d1_size <= 128) ? 128 : (_d1_size <= 256) ? 256 : 
           (_d1_size <= 512) ? 512 : (_d1_size <= 1024) ? 1024 : (_d1_size <= 2048) ? 2048 : 
           (_d1_size <= 4096) ? 4096 : (_d1_size <= 8192) ? 8192 : (_d1_size <= 16384) ? 16384 : 
           (_d1_size <= 32768) ? 32768 : (_d1_size <= 65536) ? 65536 : (_d1_size <= 131072) ? 131072 : 
           (_d1_size <= 262144) ? 262144 : 524288;
  /* Subregion size is RegionSize / 8 */
  _d1_sub = _d1_p2 / 8;
  /* Align effective size to the subregion granularity */
  _d1_pad = (_d1_size + _d1_sub - 1) / _d1_sub * _d1_sub;
  /* Advance current pointer to reserve this space */
  . = __mpu_d1_nc_start + _d1_pad;
  PROVIDE(__mpu_d1_nc_end = .);

  /* used by the startup to initialize data */
  _sidata = LOADADDR(.data);

  /* Initialized data sections goes into RAM, load LMA copy after code */
  .data :
  {
    . = ALIGN(4);
    _sdata = .;        /* create a global symbol at data start */
    *(.data)           /* .data sections */
    *(.data*)          /* .data* sections */

    . = ALIGN(4);
    _edata = .;        /* define a global symbol at data end */
  } >DTCMRAM AT> RAM_EXEC

  /* Uninitialized data section */
  . = ALIGN(4);
  .bss :
  {
    /* This is used by the startup in order to initialize the .bss section */
    _sbss = .;         /* define a global symbol at bss start */
    __bss_start__ = _sbss;
    *(.bss)
    *(.bss*)
    *(COMMON)

    . = ALIGN(4);
    _ebss = .;         /* define a global symbol at bss end */
    __bss_end__ = _ebss;
  } >DTCMRAM

  /* User_heap_stack section, used to check that there is enough RAM left */
  ._user_heap_stack :
  {
    . = ALIGN(8);
    PROVIDE ( end = . );
    PROVIDE ( _end = . );
    . = . + _Min_Heap_Size;
    . = . + _Min_Stack_Size;
    . = ALIGN(8);
  } >DTCMRAM

  /* MPU D2 Non-Cached Section: Placed at start of RAM_D2 for alignment */
  .mpu_ram_d2_nc :
  {
    . = ALIGN(32);
    PROVIDE(__mpu_d2_nc_start = .);
    *(.mpu_ram_d2_nc)
  } >RAM_D2

  /* CALCULATE PADDING FOR MPU SUBREGIONS (D2) */
  _d2_size = SIZEOF(.mpu_ram_d2_nc);
  /* Find next power of 2 */
  _d2_p2 = (_d2_size <= 32) ? 32 : (_d2_size <= 64) ? 64 : (_d2_size <= 128) ? 128 : (_d2_size <= 256) ? 256 : 
           (_d2_size <= 512) ? 512 : (_d2_size <= 1024) ? 1024 : (_d2_size <= 2048) ? 2048 : 
           (_d2_size <= 4096) ? 4096 : (_d2_size <= 8192) ? 8192 : (_d2_size <= 16384) ? 16384 : 
           (_d2_size <= 32768) ? 32768 : 65536;
  _d2_sub = _d2_p2 / 8;
  _d2_pad = (_d2_size + _d2_sub - 1) / _d2_sub * _d2_sub;
  . = __mpu_d2_nc_start + _d2_pad;
  PROVIDE(__mpu_d2_nc_end = .);

  /* MPU D3 Non-Cached Section: Placed at start of RAM_D3 for alignment */
  .mpu_ram_d3_nc :
  {
    . = ALIGN(32);
    PROVIDE(__mpu_d3_nc_start = .);
    *(.mpu_ram_d3_nc)
  } >RAM_D3

  /* CALCULATE PADDING FOR MPU SUBREGIONS (D3) */
  _d3_size = SIZEOF(.mpu_ram_d3_nc);
  /* Find next power of 2 */
  _d3_p2 = (_d3_size <= 32) ? 32 : (_d3_size <= 64) ? 64 : (_d3_size <= 128) ? 128 : (_d3_size <= 256) ? 256 : 
           (_d3_size <= 512) ? 512 : (_d3_size <= 1024) ? 1024 : (_d3_size <= 2048) ? 2048 : 
           (_d3_size <= 4096) ? 4096 : (_d3_size <= 8192) ? 8192 : (_d3_size <= 16384) ? 16384 : 32768;
  _d3_sub = _d3_p2 / 8;
  _d3_pad = (_d3_size + _d3_sub - 1) / _d3_sub * _d3_sub;
  . = __mpu_d3_nc_start + _d3_pad;
  PROVIDE(__mpu_d3_nc_end = .);

  /* Code running in ITCM RAM (0 Wait States, Instruction Bus) */
  .ram_code :
  {
  	. = ALIGN(4);
  	_sram_code = .; 
    *(.ram_code)
    *(.ram_code*)
    . = ALIGN(4);
    _eram_code = .;
  } >ITCMRAM AT> RAM_EXEC
  _siram_code = LOADADDR(.ram_code);

  /* Cached D1 Section - Explicitly placed after the Non-Cached region */
  .ram_d1 :
  {
    . = ALIGN(32);
    *(.ram_d1)
    *(.ram_d1*)
  } >RAM_D1

  /* Cached D2 Section - Explicitly placed after the Non-Cached region */
  .ram_d2 :
  {
    . = ALIGN(32);
    *(.ram_d2)
    *(.ram_d2*)
  } >RAM_D2

  /* Cached D3 Section - Explicitly placed after the Non-Cached region */
  .ram_d3 :
  {
    . = ALIGN(32);
    *(.ram_d3)
    *(.ram_d3*)
  } >RAM_D3

  /* Remove information from the standard libraries */
  /DISCARD/ :
  {
    libc.a ( * )
    libm.a ( * )
    libgcc.a ( * )
  }

  .ARM.attributes 0 : { *(.ARM.attributes) }
}
