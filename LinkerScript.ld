/* Entry Point */
ENTRY(Reset_Handler)

/* Highest address of the user mode stack */
__sstack = ORIGIN(DTCMRAM);
__estack = ORIGIN(DTCMRAM) + LENGTH(DTCMRAM);
PROVIDE(_sstack = __sstack);
PROVIDE(_estack = __estack);
/* Generate a link error if heap and stack don't fit into DTCM */
__Min_Heap_Size = 0x200;  /* required amount of heap  */
__Min_Stack_Size = 0x400; /* required amount of stack */
PROVIDE(_Min_Heap_Size = __Min_Heap_Size);
PROVIDE(_Min_Stack_Size = __Min_Stack_Size);

#if !defined(__ITCM_SIZE)
  #define __ITCM_SIZE (64K) /* Default ITCM size if not defined by the build system */
#endif
ASSERT((__ITCM_SIZE == 64K) || (__ITCM_SIZE == 128K) || (__ITCM_SIZE == 192K) || (__ITCM_SIZE == 256K), \
  "Invalid ITCM size. Must be 64K, 128K, 192K, or 256K")

#if !defined(__ITCM_BUILD)
  #define __ITCM_BUILD 0 /* By default, code is goes to just FLASH. It can be enabled via build system if needed. */
#endif

/* Specify the memory areas */
MEMORY
{
  ITCMRAM   (xrw)   : ORIGIN = 0x00000000,   LENGTH = __ITCM_SIZE /* Configurable ITCM size, default to 64K */
  DTCMRAM   (rw)    : ORIGIN = 0x20000000,   LENGTH = 128K
  FLASH     (xr)    : ORIGIN = 0x08000000,   LENGTH = 1024K - 128K - 128K
  FLASH_ST  (xr)    : ORIGIN = 0x080C0000,   LENGTH = 128K
  FLASH_BT  (xr)    : ORIGIN = 0x080E0000,   LENGTH = 128K
  RAM_D1    (rw)    : ORIGIN = 0x24000000,   LENGTH = 320K - (__ITCM_SIZE - 64K) /* ITCM memory is carved out of D1 RAM */
  RAM_D2    (rw)    : ORIGIN = 0x30000000,   LENGTH = 32K
  RAM_D3    (rw)    : ORIGIN = 0x38000000,   LENGTH = 16K
  PERIPHERAL (rw)   : ORIGIN = 0x40000000,   LENGTH = 512M
}

/* Define output sections */
SECTIONS
{
  /* Export Memory Layout Information for MPU Configuration (dynamic symbols are defined later) */
  PROVIDE(_itcm_base = ABSOLUTE(ORIGIN(ITCMRAM)));
  PROVIDE(_itcm_size = ABSOLUTE(LENGTH(ITCMRAM)));
  PROVIDE(_dtcm_base = ABSOLUTE(ORIGIN(DTCMRAM)));
  PROVIDE(_dtcm_size = ABSOLUTE(LENGTH(DTCMRAM)));
  PROVIDE(_flash_base = ABSOLUTE(ORIGIN(FLASH)));
  PROVIDE(_flash_size = ABSOLUTE(LENGTH(FLASH) + LENGTH(FLASH_ST) + LENGTH(FLASH_BT)));
  PROVIDE(_ram_d1_base = ABSOLUTE(ORIGIN(RAM_D1)));
  PROVIDE(_ram_d1_size = ABSOLUTE(LENGTH(RAM_D1)));
  PROVIDE(_ram_d2_base = ABSOLUTE(ORIGIN(RAM_D2)));
  PROVIDE(_ram_d2_size = ABSOLUTE(LENGTH(RAM_D2)));
  PROVIDE(_ram_d3_base = ABSOLUTE(ORIGIN(RAM_D3)));
  PROVIDE(_ram_d3_size = ABSOLUTE(LENGTH(RAM_D3)));
  PROVIDE(_peripheral_base = ABSOLUTE(ORIGIN(PERIPHERAL)));
  PROVIDE(_peripheral_size = ABSOLUTE(LENGTH(PERIPHERAL)));

  /* Null-pointer guard: MPU blocks first 32 bytes. VTOR requires 128-byte alignment. */
  .null_guard (NOLOAD) :
  {
    . += 32;
    . = ALIGN(1024); // 156 vectors * 4 bytes each = 624 bytes, next power of 2 is 1024
  } >ITCMRAM

  /* The interrupt vector table goes into ITCM RAM */
  .isr_vector :
  {
    . = ALIGN(4);
    __sisr_vector = ABSOLUTE(.);
    KEEP(*(.isr_vector)) /* Interrupt vector table */

    . = ALIGN(4);
    __eisr_vector = ABSOLUTE(.);
  } >ITCMRAM AT>FLASH
   __si_isr_vector = LOADADDR(.isr_vector);

  /* This section ALWAYS stays in Flash */
  .boot :
  {
    . = ALIGN(4);
    *(.boot_code*)    /* Everything tagged with .boot_code stays here */
    . = ALIGN(4);
  } >FLASH

  /* The program code goes into FLASH */
  .text :
  {
    . = ALIGN(4);
    __stext = ABSOLUTE(.);
    PROVIDE(_stext = __stext);

#if !__ITCM_BUILD
    *(.text)          /* .text sections (code) */
    *(.text*)         /* .text* sections (code) */
    *(.glue_7)        /* glue arm to thumb code */
    *(.glue_7t)       /* glue thumb to arm code */
    *(.eh_frame)      /* Exception handling frames, mainly for the debugger */
#endif

    KEEP(*(.init))
    KEEP(*(.fini))

    . = ALIGN(4);
    __etext = ABSOLUTE(.);
    PROVIDE(_etext = __etext);
  } >FLASH

  /* Code running in ITCM RAM (0 Wait States, Instruction Bus) */
  .ram_code :
  {
  	. = ALIGN(4);
  	__sram_code = ABSOLUTE(.);
    *(.ram_code)    /* .ram_code sections, for code marked to be placed in ITCM RAM */
    *(.ram_code*)   /* .ram_code* sections, for code marked to be placed in ITCM RAM */

#if __ITCM_BUILD
    *(.text)          /* .text sections (code) */
    *(.text*)         /* .text* sections (code) */
    *(.glue_7)        /* glue arm to thumb code */
    *(.glue_7t)       /* glue thumb to arm code */
    *(.eh_frame)      /* Exception handling frames, mainly for the debugger */
#endif

    . = ALIGN(4);
    __eram_code = ABSOLUTE(.);
  } >ITCMRAM AT> FLASH
  __si_ram_code = LOADADDR(.ram_code);

  /* Constant data goes into FLASH */
  .rodata :
  {
    . = ALIGN(4);
    *(.rodata)        /* .rodata sections (constants, strings, etc.) */
    *(.rodata*)       /* .rodata* sections (constants, strings, etc.) */

    . = ALIGN(4);
  } >FLASH

  /* ARM exception handling tables go into FLASH */
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

  /* .hard_fault_log has to be the first thing in the FLASH_ST */
  .hardfault_log :
  {
    . = ALIGN(4);
    __hf_log = .;
    KEEP(*(.hardfault_log));
    . += 0x200;

    . = ALIGN(4);
    PROVIDE(_hf_log = __hf_log);
  } >FLASH_ST

  .metadata_pool :
  {
    . = ALIGN(4);
    __metadata = .;
    KEEP(*(.metadata_pool))
    . += 0x100;

    . = ALIGN(4);
    PROVIDE(_metadata = __metadata);
  } >FLASH_ST

  /* Stack memory to avoid memfault inside hardfault handler, at the start of DTCM */
  .hardfault_stack (NOLOAD) :
  {
    . = ALIGN(8);
    __hf_stack_start = .;
    PROVIDE(_hf_stack_start = __hf_stack_start);
    . += 0x400;       /* 1 KB */

    . = ALIGN(8);
    __hf_stack_end = .;
    PROVIDE(_hf_stack_end = __hf_stack_end);
  } >DTCMRAM

  /* Initialized data sections goes into RAM, copied from FLASH */
  .data :
  {
    . = ALIGN(4);
    __sdata = .;
    *(.data)          /* .data sections */
    *(.data*)         /* .data* sections */

    . = ALIGN(4);
    __edata = .;
  } >DTCMRAM AT> FLASH
  __si_data = LOADADDR(.data);

  .dtcm_rodata :
  {
    . = ALIGN(4);
    __sdtcm_rodata = .;
    *(.dtcm.rodata)   /* .dtcm_rodata sections, for constants marked to be placed in DTCM RAM */
    *(.dtcm.rodata*)  /* .dtcm_rodata* sections, for constants marked to be placed in DTCM RAM */
    BYTE(0);

    . = ALIGN(4);
    __edtcm_rodata = .;
  } >DTCMRAM AT> FLASH
  __si_dtcm_rodata = LOADADDR(.dtcm_rodata);

  /* Uninitialized data section */
  .bss (NOLOAD) :
  {
    . = ALIGN(4);
    __sbss = .;
    __bss_start__ = __sbss;
    *(.bss)           /* .bss sections */
    *(.bss*)          /* .bss* sections */
    *(COMMON)         /* common sections, same as the bss, mainly generated by legacy code */

    . = ALIGN(4);
    __ebss = .;
    __bss_end__ = __ebss;
  } >DTCMRAM

  /* User_heap_stack section, used to check that there is enough RAM left */
  ._user_heap_stack (NOLOAD) :
  {
    . = ALIGN(8);
    PROVIDE ( end = . );
    PROVIDE ( _end = . );
    . = . + __Min_Heap_Size;
    . = . + __Min_Stack_Size;

    . = ALIGN(8);
  } >DTCMRAM

  /* MPU D1 Non-Cached Section: Placed at start of RAM_D1 for alignment */
  .ram_d1_nc_bss (NOLOAD) :
  {
    . = ALIGN(32);
    __d1_nc_start = ABSOLUTE(.);
    __sram_d1_nc_bss = ABSOLUTE(.);

    /* MPU system buffers */
    *(.ram_d1_nc.buffer)
    *(.ram_d1_nc.buffer*)

    /* User manual allocations via D1_NC macro */
    . = ALIGN(32);
    *(.ram_d1_nc.user.bss)
    *(.ram_d1_nc.user.bss*)

    /* Ethernet Rx Pool */
    . = ALIGN(32);
    *(.Rx_PoolSection)

    __eram_d1_nc_bss = ABSOLUTE(.);
  } >RAM_D1
  .ram_d1_nc_data_rodata :
  {
    . = ALIGN(32);
    __sram_d1_nc_data = ABSOLUTE(.);

    *(.ram_d1_nc.user.data)
    *(.ram_d1_nc.user.data*)
    *(.ram_d1_nc.user.rodata)
    *(.ram_d1_nc.user.rodata*)
    BYTE(0);

    . = ALIGN(4);
    __eram_d1_nc_data = ABSOLUTE(.);
  } >RAM_D1 AT> FLASH
  __si_d1_nc_data = LOADADDR(.ram_d1_nc_data_rodata);
  /* Calculate padding for MPU subregions (D1) */
  __d1_size = . - __d1_nc_start;
  /* Find next power of 2 (up to 512KB for 320KB RAM) */
  __d1_p2 = (1 << LOG2CEIL(MAX(32, __d1_size)));
  /* Subregion size is RegionSize / 8 */
  __d1_sub = __d1_p2 / 8;
  /* Align effective size to the subregion granularity */
  __d1_pad = (__d1_size + __d1_sub - 1) / __d1_sub * __d1_sub;
  /* Reserve MPU-aligned padding between non-cached and cached regions */
  .ram_d1_nc_pad (NOLOAD) :
  {
    . += (__d1_pad - __d1_size);
  } >RAM_D1
  __d1_nc_end = ABSOLUTE(.);
  PROVIDE(_ram_d1_nc_start = __d1_nc_start);
  PROVIDE(_ram_d1_nc_end = __d1_nc_end);

  /* MPU D1 Cached Section */
  .ram_d1_bss (NOLOAD) :
  {
    . = ALIGN(32);
    __d1_start = ABSOLUTE(.);
    __sram_d1_bss = ABSOLUTE(.);

    /* MPU system buffers */
    *(.ram_d1.buffer)
    *(.ram_d1.buffer*)

    /* User manual allocations via D1_C macro */
    . = ALIGN(32);
    *(.ram_d1.user.bss)
    *(.ram_d1.user.bss*)

    __eram_d1_bss = ABSOLUTE(.);
  } >RAM_D1
  .ram_d1_data_rodata :
  {
    . = ALIGN(32);
    __sram_d1_data = ABSOLUTE(.);

    *(.ram_d1.user.data)
    *(.ram_d1.user.data*)
    *(.ram_d1.user.rodata)
    *(.ram_d1.user.rodata*)
    BYTE(0);

    . = ALIGN(4);
    __eram_d1_data = ABSOLUTE(.);
    __d1_end = ABSOLUTE(.);
  } >RAM_D1 AT> FLASH
  __si_d1_data = LOADADDR(.ram_d1_data_rodata);

  /* MPU D2 Non-Cached Section: Contains Ethernet Descriptors and Generic Buffers */
  .ram_d2_nc_bss (NOLOAD) :
  {
    . = ALIGN(32);
    __d2_nc_start = ABSOLUTE(.);
    __sram_d2_nc_bss = ABSOLUTE(.);

    /* ETH Descriptors - Must be aligned */
    *(.RxDecripSection)
    *(.TxDecripSection)

    /* MPU system buffers */
    *(.ram_d2_nc.buffer)
    *(.ram_d2_nc.buffer*)

    /* User manual allocations via D2_NC macro */
    . = ALIGN(32);
    *(.ram_d2_nc.user.bss)
    *(.ram_d2_nc.user.bss*)
    __eram_d2_nc_bss = ABSOLUTE(.);
  } >RAM_D2
  .ram_d2_nc_data_rodata :
  {
    . = ALIGN(32);
    __sram_d2_nc_data = ABSOLUTE(.);

    *(.ram_d2_nc.user.data)
    *(.ram_d2_nc.user.data*)
    *(.ram_d2_nc.user.rodata)
    *(.ram_d2_nc.user.rodata*)
    BYTE(0);

    . = ALIGN(4);
    __eram_d2_nc_data = ABSOLUTE(.);
  } >RAM_D2 AT> FLASH
  __si_d2_nc_data = LOADADDR(.ram_d2_nc_data_rodata);
  /* Calculate padding for MPU subregions (D2) */
  __d2_size = . - __d2_nc_start;
  /* Find next power of 2 (up to 64KB for 32KB RAM) */
  __d2_p2 = (1 << LOG2CEIL(MAX(32, __d2_size)));
  __d2_sub = __d2_p2 / 8;
  __d2_pad = (__d2_size + __d2_sub - 1) / __d2_sub * __d2_sub;
  .ram_d2_nc_pad (NOLOAD) :
  {
    . += (__d2_pad - __d2_size);
  } >RAM_D2
  __d2_nc_end = ABSOLUTE(.);
  PROVIDE(_ram_d2_nc_start = __d2_nc_start);
  PROVIDE(_ram_d2_nc_end = __d2_nc_end);

  /* MPU D2 Cached Section */
  .ram_d2_bss (NOLOAD) :
  {
    . = ALIGN(32);
    __d2_start = ABSOLUTE(.);
    __sram_d2_bss = ABSOLUTE(.);

    /* MPU system buffers */
    *(.ram_d2.buffer)
    *(.ram_d2.buffer*)

    /* User manual allocations via D2_C macro */
    . = ALIGN(32);
    *(.ram_d2.user.bss)
    *(.ram_d2.user.bss*)

    __eram_d2_bss = ABSOLUTE(.);
  } >RAM_D2
  .ram_d2_data_rodata :
  {
    . = ALIGN(32);
    __sram_d2_data = ABSOLUTE(.);

    *(.ram_d2.user.data)
    *(.ram_d2.user.data*)
    *(.ram_d2.user.rodata)
    *(.ram_d2.user.rodata*)
    BYTE(0);

    . = ALIGN(4);
    __eram_d2_data = ABSOLUTE(.);
    __d2_end = ABSOLUTE(.);
  } >RAM_D2 AT> FLASH
  __si_d2_data = LOADADDR(.ram_d2_data_rodata);

  /* MPU D3 Non-Cached Section */
  .ram_d3_nc_bss (NOLOAD) :
  {
    . = ALIGN(32);
    __d3_nc_start = ABSOLUTE(.);
    __sram_d3_nc_bss = ABSOLUTE(.);

    /* MPU system buffers */
    *(.ram_d3_nc.buffer)
    *(.ram_d3_nc.buffer*)

    /* Legacy MPUManager allocations */
    . = ALIGN(32);
    *(.ram_d3_nc.legacy)

    /* User manual allocations via D3_NC macro */
    . = ALIGN(32);
    *(.ram_d3_nc.user.bss)
    *(.ram_d3_nc.user.bss*)
    __eram_d3_nc_bss = ABSOLUTE(.);
  } >RAM_D3
  .ram_d3_nc_data_rodata :
  {
    . = ALIGN(32);
    __sram_d3_nc_data = ABSOLUTE(.);

    *(.ram_d3_nc.user.data)
    *(.ram_d3_nc.user.data*)
    *(.ram_d3_nc.user.rodata)
    *(.ram_d3_nc.user.rodata*)
    BYTE(0);

    . = ALIGN(4);
    __eram_d3_nc_data = ABSOLUTE(.);
  } >RAM_D3 AT> FLASH
  __si_d3_nc_data = LOADADDR(.ram_d3_nc_data_rodata);
  /* Calculate padding for MPU subregions (D3) */
  __d3_size = . - __d3_nc_start;
  /* Find next power of 2 (up to 32KB for 16KB RAM) */
  __d3_p2 = (1 << LOG2CEIL(MAX(32, __d3_size)));
  __d3_sub = __d3_p2 / 8;
  __d3_pad = (__d3_size + __d3_sub - 1) / __d3_sub * __d3_sub;
  .ram_d3_nc_pad (NOLOAD) :
  {
    . += (__d3_pad - __d3_size);
  } >RAM_D3
  __d3_nc_end = ABSOLUTE(.);
  PROVIDE(_ram_d3_nc_start = __d3_nc_start);
  PROVIDE(_ram_d3_nc_end = __d3_nc_end);

  /* MPU D3 Cached Section */
  .ram_d3_bss (NOLOAD) :
  {
    . = ALIGN(32);
    __d3_start = ABSOLUTE(.);
    __sram_d3_bss = ABSOLUTE(.);

    /* MPU system buffers */
    *(.ram_d3.buffer)
    *(.ram_d3.buffer*)

    /* User manual allocations via D3_C macro */
    . = ALIGN(32);
    *(.ram_d3.user.bss)
    *(.ram_d3.user.bss*)

    __eram_d3_bss = ABSOLUTE(.);
  } >RAM_D3
  .ram_d3_data_rodata :
  {
    . = ALIGN(32);
    __sram_d3_data = ABSOLUTE(.);

    *(.ram_d3.user.data)
    *(.ram_d3.user.data*)
    *(.ram_d3.user.rodata)
    *(.ram_d3.user.rodata*)
    BYTE(0);

    . = ALIGN(4);
    __eram_d3_data = ABSOLUTE(.);
    __d3_end = ABSOLUTE(.);
  } >RAM_D3 AT> FLASH
  __si_d3_data = LOADADDR(.ram_d3_data_rodata);

  /* Table of sections to be copied from Flash to RAM */
  .copy_table :
  {
    . = ALIGN(4);
    __copy_table_start = .;

    /* [Source LMA,         Dest VMA Start,       Dest VMA End] */
    LONG(__si_isr_vector);  LONG(__sisr_vector);  LONG(__eisr_vector);
    LONG(__si_ram_code);    LONG(__sram_code);    LONG(__eram_code);
    LONG(__si_data);        LONG(__sdata);        LONG(__edata);
    LONG(__si_dtcm_rodata); LONG(__sdtcm_rodata); LONG(__edtcm_rodata);
    LONG(__si_d1_nc_data); LONG(__sram_d1_nc_data); LONG(__eram_d1_nc_data);
    LONG(__si_d1_data);     LONG(__sram_d1_data);     LONG(__eram_d1_data);
    LONG(__si_d2_nc_data); LONG(__sram_d2_nc_data); LONG(__eram_d2_nc_data);
    LONG(__si_d2_data);     LONG(__sram_d2_data);     LONG(__eram_d2_data);
    LONG(__si_d3_nc_data); LONG(__sram_d3_nc_data); LONG(__eram_d3_nc_data);
    LONG(__si_d3_data);     LONG(__sram_d3_data);     LONG(__eram_d3_data);

    __copy_table_end = .;
  } > FLASH

  /* Table of sections to be zero-initialized in RAM */
  .zero_table :
  {
    . = ALIGN(4);
    __zero_table_start = .;

    /* [Dest VMA Start,     Dest VMA End] */
    LONG(__sbss);           LONG(__ebss);
    LONG(__sram_d1_nc_bss); LONG(__eram_d1_nc_bss);
    LONG(__sram_d1_bss);    LONG(__eram_d1_bss);
    LONG(__sram_d2_nc_bss); LONG(__eram_d2_nc_bss);
    LONG(__sram_d2_bss);    LONG(__eram_d2_bss);
    LONG(__sram_d3_nc_bss); LONG(__eram_d3_nc_bss);
    LONG(__sram_d3_bss);    LONG(__eram_d3_bss);

    __zero_table_end = .;
  } > FLASH

  /* Remove information from the standard libraries */
  /DISCARD/ :
  {
    libc.a ( * )
    libm.a ( * )
    libgcc.a ( * )
  }

  .ARM.attributes 0 : { *(.ARM.attributes) } /* Metadata, not allocated, only exists in the .elf */
}
