/**
  ******************************************************************************
  * STM32F746NGHx Linker Script for Firmware
  * FLASH: 1024K | RAM: 320K | CCM: 64K
  ******************************************************************************
  */

MEMORY
{
  FLASH (rx)  : ORIGIN = 0x08000000, LENGTH = 1024K
  RAM (rwx)   : ORIGIN = 0x20000000, LENGTH = 320K
  CCM (rwx)   : ORIGIN = 0x20050000, LENGTH = 64K
}

SECTIONS
{
  .isr_vector :
  {
    . = ALIGN(4);
    KEEP(*(.isr_vector))
    . = ALIGN(4);
  } >FLASH

  .text :
  {
    . = ALIGN(4);
    *(.text)
    *(.text*)
    *(.rodata)
    *(.rodata*)
    . = ALIGN(4);
  } >FLASH

  _etext = .;

  .data : 
  {
    . = ALIGN(4);
    _sdata = .;
    *(.data)
    *(.data*)
    . = ALIGN(4);
    _edata = .;
  } >RAM AT >FLASH

  _sidata = LOADADDR(.data);

  .bss :
  {
    . = ALIGN(4);
    _sbss = .;
    *(.bss)
    *(.bss*)
    *(COMMON)
    . = ALIGN(4);
    _ebss = .;
  } >RAM

  .ccm :
  {
    . = ALIGN(4);
    *(.ccm)
    . = ALIGN(4);
  } >CCM AT >FLASH

  .ARM.attributes 0 : { *(.ARM.attributes) }
}
