/* Linker script for STM32F407VG (1MB FLASH, 128KB SRAM, 64KB CCMRAM) */
MEMORY
{
  FLASH (rx)      : ORIGIN = 0x08000000, LENGTH = 1024K
  RAM (xrw)       : ORIGIN = 0x20000000, LENGTH = 128K
  CCMRAM (xrw)    : ORIGIN = 0x10000000, LENGTH = 64K
}

_estack = ORIGIN(RAM) + LENGTH(RAM);

/* Sections */
SECTIONS
{
  .isr_vector :
  {
    . = ALIGN(4);
    KEEP(*(.isr_vector))
    . = ALIGN(4);
  } > FLASH

  .text :
  {
    . = ALIGN(4);
    *(.text) *(.text*)
    *(.rodata) *(.rodata*)
    *(.glue_7) *(.glue_7t)
    *(.eh_frame)
    . = ALIGN(4);
  } > FLASH

  .ARM.extab   : { *(.ARM.extab* .gnu.linkonce.armextab.*) } > FLASH
  .ARM.exidx   : { *(.ARM.exidx* .gnu.linkonce.armexidx.*) } > FLASH

  _sidata = LOADADDR(.data);

  .data : AT (ADDR(.text) + SIZEOF(.text))
  {
    . = ALIGN(4);
    _sdata = .;        /* start of .data in RAM */
    *(.data) *(.data*)
    . = ALIGN(4);
    _edata = .;        /* end of .data in RAM */
  } > RAM

  .bss :
  {
    . = ALIGN(4);
    _sbss = .;         /* start of .bss in RAM */
    *(.bss) *(.bss*) *(COMMON)
    . = ALIGN(4);
    _ebss = .;         /* end of .bss in RAM */
  } > RAM

  /* Optional: CCMRAM for fast data/code if needed */
  .ccmram (NOLOAD) :
  {
    . = ALIGN(4);
    *(.ccmram) *(.ccmram*)
    . = ALIGN(4);
  } > CCMRAM

  /* Heap and stack */
  ._user_heap_stack :
  {
    . = ALIGN(8);
    PROVIDE(end = .);
    PROVIDE(_end = .);
    . = . + 0x2000; /* 8KB heap placeholder */
    . = ALIGN(8);
  } > RAM
}
