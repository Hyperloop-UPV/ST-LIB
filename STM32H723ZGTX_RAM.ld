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

/* This linker script places code and read-only data in ITCM RAM for execution
   and places initialized data in DTCM RAM. This only leaves 64kB for code, so
   it could potentially overflow, if you want to host a big application, maybe
   should use RAM_D1 for code as well (and either adjust MPU settings or mess
   arround with the memory areas specifications, so that the code thinks RAM_D1
   is actually FLASH). */

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
  __itcm_base = ABSOLUTE(ORIGIN(ITCMRAM));
  __itcm_size = ABSOLUTE(LENGTH(ITCMRAM));
  __dtcm_base = ABSOLUTE(ORIGIN(DTCMRAM));
  __dtcm_size = ABSOLUTE(LENGTH(DTCMRAM));
  __flash_base = ABSOLUTE(ORIGIN(FLASH));
  __flash_size = ABSOLUTE(LENGTH(FLASH));
  __ram_d1_base = ABSOLUTE(ORIGIN(RAM_D1));
  __ram_d1_size = ABSOLUTE(LENGTH(RAM_D1));
  __ram_d2_base = ABSOLUTE(ORIGIN(RAM_D2));
  __ram_d2_size = ABSOLUTE(LENGTH(RAM_D2));
  __ram_d3_base = ABSOLUTE(ORIGIN(RAM_D3));
  __ram_d3_size = ABSOLUTE(LENGTH(RAM_D3));
  __peripheral_base = 0x40000000;
  __peripheral_size = 0x20000000; /* 512MB */

  /* The startup code goes into ITCM for fastest access */
  .isr_vector :
  {
    . = ALIGN(4);
    KEEP(*(.isr_vector)) /* Startup code */
    . = ALIGN(4);
  } >ITCMRAM

  /* The program code goes into ITCM (instruction-optimized, 0 wait states) */
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
  } >ITCMRAM

  /* Constant data goes into RAM_D1 (ITCM is too small for rodata) */
  .rodata :
  {
    . = ALIGN(4);
    *(.rodata)         /* .rodata sections (constants, strings, etc.) */
    *(.rodata*)        /* .rodata* sections (constants, strings, etc.) */
    . = ALIGN(4);
  } >RAM_D1

  .ARM.extab (READONLY): { *(.ARM.extab* .gnu.linkonce.armextab.*) } >RAM_D1
  .ARM (READONLY): {
    __exidx_start = .;
    *(.ARM.exidx*)
    __exidx_end = .;
  } >RAM_D1

  .preinit_array (READONLY):
  {
    PROVIDE_HIDDEN (__preinit_array_start = .);
    KEEP (*(.preinit_array*))
    PROVIDE_HIDDEN (__preinit_array_end = .);
  } >RAM_D1

  .init_array (READONLY):
  {
    PROVIDE_HIDDEN (__init_array_start = .);
    KEEP (*(SORT(.init_array.*)))
    KEEP (*(.init_array*))
    PROVIDE_HIDDEN (__init_array_end = .);
  } >RAM_D1

  .fini_array (READONLY):
  {
    PROVIDE_HIDDEN (__fini_array_start = .);
    KEEP (*(SORT(.fini_array.*)))
    KEEP (*(.fini_array*))
    PROVIDE_HIDDEN (__fini_array_end = .);
  } >RAM_D1

  /* MPU D1 Non-Cached Section */
  .mpu_ram_d1_nc :
  {
    . = ALIGN(32);
    __mpu_d1_nc_start = ABSOLUTE(.);

    /* New MPU system buffers */
    *(.mpu_ram_d1_nc.buffer)

    /* User manual allocations via D1_NC macro */
    *(.mpu_ram_d1_nc.user)

    /* Ethernet Rx Pool */
    . = ALIGN(32);
    *(.Rx_PoolSection)
  } >RAM_D1

  /* CALCULATE PADDING FOR MPU SUBREGIONS (D1) */
  _d1_size = SIZEOF(.mpu_ram_d1_nc);
  /* Find next power of 2 */
  _d1_p2 = (1 << LOG2CEIL(MAX(32, _d1_size)));
  /* Subregion size is RegionSize / 8 */
  _d1_sub = _d1_p2 / 8;
  /* Align effective size to the subregion granularity */
  _d1_pad = (_d1_size + _d1_sub - 1) / _d1_sub * _d1_sub;
  /* Advance current pointer to reserve this space */
  . = __mpu_d1_nc_start + _d1_pad;
  __mpu_d1_nc_end = ABSOLUTE(.);


  /* Initialized data sections goes into DTCM */
  .data :
  {
    . = ALIGN(4);
    _sdata = .;        /* create a global symbol at data start */
    *(.data)           /* .data sections */
    *(.data*)          /* .data* sections */

    . = ALIGN(4);
    _edata = .;        /* define a global symbol at data end */
  } >DTCMRAM

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

  /* MPU D2 Non-Cached Section */
  .mpu_ram_d2_nc :
  {
    . = ALIGN(32);
    __mpu_d2_nc_start = ABSOLUTE(.);

    /* Ethernet descriptors */
    *(.RxDecripSection)
    *(.TxDecripSection)

    /* New MPU system buffers */
    *(.mpu_ram_d2_nc.buffer)

    /* User manual allocations via D2_NC macro */
    *(.mpu_ram_d2_nc.user)
  } >RAM_D2

  /* CALCULATE PADDING FOR MPU SUBREGIONS (D2) */
  _d2_size = SIZEOF(.mpu_ram_d2_nc);
  /* Find next power of 2 */
  _d2_p2 = (1 << LOG2CEIL(MAX(32, _d2_size)));
  _d2_sub = _d2_p2 / 8;
  _d2_pad = (_d2_size + _d2_sub - 1) / _d2_sub * _d2_sub;
  . = __mpu_d2_nc_start + _d2_pad;
  __mpu_d2_nc_end = ABSOLUTE(.);

  /* MPU D3 Non-Cached Section */
  .mpu_ram_d3_nc :
  {
    . = ALIGN(32);
    __mpu_d3_nc_start = ABSOLUTE(.);

    /* New MPU system buffers */
    *(.mpu_ram_d3_nc.buffer)

    /* Legacy MPUManager allocations */
    *(.mpu_ram_d3_nc.legacy)

    /* User manual allocations via D3_NC macro */
    *(.mpu_ram_d3_nc.user)
  } >RAM_D3

  /* CALCULATE PADDING FOR MPU SUBREGIONS (D3) */
  _d3_size = SIZEOF(.mpu_ram_d3_nc);
  /* Find next power of 2 */
  _d3_p2 = (1 << LOG2CEIL(MAX(32, _d3_size)));
  _d3_sub = _d3_p2 / 8;
  _d3_pad = (_d3_size + _d3_sub - 1) / _d3_sub * _d3_sub;
  . = __mpu_d3_nc_start + _d3_pad;
  __mpu_d3_nc_end = ABSOLUTE(.);

  /* Additional code in ITCM, doesn't really do much with this configuration since everything is in ITCM anyways */
  .ram_code :
  {
  	. = ALIGN(4);
  	_sram_code = .;
    *(.ram_code)
    *(.ram_code*)
    . = ALIGN(4);
    _eram_code = .;
  } >ITCMRAM

  /* MPU D1 Cached Section */
  .ram_d1 :
  {
    . = ALIGN(32);

    /* New MPU system buffers */
    *(.ram_d1.buffer)

    /* User manual allocations via D1_C macro */
    *(.ram_d1.user)
  } >RAM_D1

  /* MPU D2 Cached Section */
  .ram_d2 :
  {
    . = ALIGN(32);

    /* New MPU system buffers */
    *(.ram_d2.buffer)

    /* User manual allocations via D2_C macro */
    *(.ram_d2.user)
  } >RAM_D2

  /* MPU D3 Cached Section */
  .ram_d3 :
  {
    . = ALIGN(32);

    /* New MPU system buffers */
    *(.ram_d3.buffer)

    /* User manual allocations via D3_C macro */
    *(.ram_d3.user)
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
