/*
******************************************************************************
**
**  File        : LinkerScript.ld
**
**  Author      : STM32CubeIDE
**
**  Abstract    : Linker script for STM32H7 series
**                1024Kbytes FLASH and 560Kbytes RAM
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
/* Generate a link error if heap and stack don't fit into RAM/DTCM */
_Min_Heap_Size = 0x200;      /* required amount of heap  */
_Min_Stack_Size = 0x400; /* required amount of stack */

/* Specify the memory areas */
MEMORY
{
  ITCMRAM (xrw)    : ORIGIN = 0x00000000,   LENGTH = 64K
  DTCMRAM (xrw)    : ORIGIN = 0x20000000,   LENGTH = 128K
  FLASH    (rx)    : ORIGIN = 0x08000000,   LENGTH = 1024K-128K
  RAM_D1  (xrw)    : ORIGIN = 0x24000000,   LENGTH = 320K
  RAM_D2  (xrw)    : ORIGIN = 0x30000000,   LENGTH = 32K
  RAM_D3  (xrw)    : ORIGIN = 0x38000000,   LENGTH = 16K
}

/* Define output sections */
SECTIONS
{
  /* Export Memory Layout Information for MPU Configuration */
  PROVIDE(__itcm_base = ORIGIN(ITCMRAM));
  PROVIDE(__itcm_size = LENGTH(ITCMRAM));
  PROVIDE(__dtcm_base = ORIGIN(DTCMRAM));
  PROVIDE(__dtcm_size = LENGTH(DTCMRAM));
  PROVIDE(__flash_base = ORIGIN(FLASH));
  PROVIDE(__flash_size = LENGTH(FLASH));
  PROVIDE(__ram_d1_base = ORIGIN(RAM_D1));
  PROVIDE(__ram_d1_size = LENGTH(RAM_D1));
  PROVIDE(__ram_d2_base = ORIGIN(RAM_D2));
  PROVIDE(__ram_d2_size = LENGTH(RAM_D2));
  PROVIDE(__ram_d3_base = ORIGIN(RAM_D3));
  PROVIDE(__ram_d3_size = LENGTH(RAM_D3));
  PROVIDE(__peripheral_base = 0x40000000);
  PROVIDE(__peripheral_size = 0x20000000); /* 512MB */

  /* The startup code goes first into FLASH */
  .isr_vector :
  {
    . = ALIGN(4);
    KEEP(*(.isr_vector)) /* Startup code */
    . = ALIGN(4);
  } >FLASH

  /* The program code and other data goes into FLASH */
  .text :
  {
    . = ALIGN(4);
    *(.text)           /* .text sections (code) */
    *(.text*)          /* .text* sections (code) */
    *(.glue_7)         /* glue arm to thumb code */
    *(.glue_7t)        /* glue thumb to arm code */
    *(.eh_frame)

    KEEP (*(.init))
    KEEP (*(.fini))

    . = ALIGN(4);
    _etext = .;        /* define a global symbols at end of code */
  } >FLASH

  /* Constant data goes into FLASH */
  .rodata :
  {
    . = ALIGN(4);
    *(.rodata)         /* .rodata sections (constants, strings, etc.) */
    *(.rodata*)        /* .rodata* sections (constants, strings, etc.) */
    . = ALIGN(4);
  } >FLASH

  .ARM.extab (READONLY): { *(.ARM.extab* .gnu.linkonce.armextab.*) } >FLASH
  .ARM (READONLY): {
    __exidx_start = .;
    *(.ARM.exidx*)
    __exidx_end = .;
  } >FLASH

  .preinit_array (READONLY):
  {
    PROVIDE_HIDDEN (__preinit_array_start = .);
    KEEP (*(.preinit_array*))
    PROVIDE_HIDDEN (__preinit_array_end = .);
  } >FLASH

  .init_array (READONLY):
  {
    PROVIDE_HIDDEN (__init_array_start = .);
    KEEP (*(SORT(.init_array.*)))
    KEEP (*(.init_array*))
    PROVIDE_HIDDEN (__init_array_end = .);
  } >FLASH

  .fini_array (READONLY):
  {
    PROVIDE_HIDDEN (__fini_array_start = .);
    KEEP (*(SORT(.fini_array.*)))
    KEEP (*(.fini_array*))
    PROVIDE_HIDDEN (__fini_array_end = .);
  } >FLASH

  /* MPU D1 Non-Cached Section: Placed at start of RAM_D1 for alignment */
  .mpu_ram_d1_nc :
  {
    . = ALIGN(32);
    PROVIDE(__mpu_d1_nc_start = .);
    *(.mpu_ram_d1_nc)
    . = ALIGN(32);
    *(.Rx_PoolSection)
  } >RAM_D1

  /* CALCULATE PADDING FOR MPU SUBREGIONS (D1) */
  _d1_size = SIZEOF(.mpu_ram_d1_nc);
  /* Find next power of 2 (up to 512KB for 320KB RAM) */
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

    /* No implementation nor use for them just yet
    *(.RamFunc)        /* .RamFunc sections */
    *(.RamFunc*)       /* .RamFunc* sections */
  */

    . = ALIGN(4);
    _edata = .;        /* define a global symbol at data end */
  } >DTCMRAM AT> FLASH
  /*
    this needs to be the last thing in FLASH
    because the preceeding sections are appended after the one preceeding them
    this is, if this were the first thing in FLASH
    the sections below it would try to be placed afterwards
    thus overflowing the FLASH
  */
  .metadata_pool :
  {
  	. = ABSOLUTE(0x080DFD00);
    . = ALIGN(4);
    metadata = .;
    KEEP(*(.metadata_pool))
    . += 0x100;
  } >FLASH
  /* Uninitialized data section */
  . = ALIGN(4);
  .bss (NOLOAD) :
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

  /* MPU D2 Non-Cached Section: Contains Ethernet Descriptors and Generic Buffers */
  .mpu_ram_d2_nc :
  {
    . = ALIGN(32);
    PROVIDE(__mpu_d2_nc_start = .);
    
    /* ETH Descriptors - Must be aligned */
    *(.RxDecripSection) 
    *(.TxDecripSection)
    
    /* Generic Non-Cached D2 data */
    *(.mpu_ram_d2_nc)
  } >RAM_D2

  /* CALCULATE PADDING FOR MPU SUBREGIONS (D2) */
  _d2_size = SIZEOF(.mpu_ram_d2_nc);
  /* Find next power of 2 (up to 64KB for 32KB RAM to catch overflow logic if needed) */
  _d2_p2 = (_d2_size <= 32) ? 32 : (_d2_size <= 64) ? 64 : (_d2_size <= 128) ? 128 : (_d2_size <= 256) ? 256 : 
           (_d2_size <= 512) ? 512 : (_d2_size <= 1024) ? 1024 : (_d2_size <= 2048) ? 2048 : 
           (_d2_size <= 4096) ? 4096 : (_d2_size <= 8192) ? 8192 : (_d2_size <= 16384) ? 16384 : 
           (_d2_size <= 32768) ? 32768 : 65536;
  _d2_sub = _d2_p2 / 8;
  _d2_pad = (_d2_size + _d2_sub - 1) / _d2_sub * _d2_sub;
  . = __mpu_d2_nc_start + _d2_pad;
  PROVIDE(__mpu_d2_nc_end = .);

  /* MPU D3 Non-Cached Section */
  .mpu_ram_d3_nc :
  {
    . = ALIGN(32);
    PROVIDE(__mpu_d3_nc_start = .);
    *(.mpu_ram_d3_nc)
  } >RAM_D3

  /* CALCULATE PADDING FOR MPU SUBREGIONS (D3) */
  _d3_size = SIZEOF(.mpu_ram_d3_nc);
  /* Find next power of 2 (up to 32KB for 16KB RAM) */
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
  } >ITCMRAM AT> FLASH
  _siram_code = LOADADDR(.ram_code);

  /* Remove information from the standard libraries */
  /DISCARD/ :
  {
    libc.a ( * )
    libm.a ( * )
    libgcc.a ( * )
  }

  .ARM.attributes 0 : { *(.ARM.attributes) }
}
