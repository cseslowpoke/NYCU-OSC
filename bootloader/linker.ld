SECTIONS
{
  . = 0x80000;
  .text.relo : { KEEP(*(.text.relo))}
  _boot_begin = .;
  .text.boot :  { KEEP(*(.text.boot)) }
  .text : { *(.text) }

  .rodata : ALIGN(8) {
      *(.rodata*)
      . = ALIGN(8);
  }
  . = ALIGN(8);

  .data : { *(.data) }

  _bootloader_size = . - _boot_begin;
  /* record start/end of bss section to fill 0 */
  . = 0x40000 + (. - _boot_begin);
  . = ALIGN(8);
  _bss_begin = .; 
  .bss (NOLOAD): { *(.bss) }
  _bss_end = .; 

  _stack_top = .  + 0x8000; /* 32 KB stack */
}
