SECTIONS
{
  . = 0x80000;
  .text.boot : { KEEP(*(.text.boot)) }
  .text : { *(.text) }

  .rodata : ALIGN(8) {
      *(.rodata*)
      . = ALIGN(8);
  }
  . = ALIGN(8);

  .data : { *(.data) }

  /* record start/end of bss section to fill 0 */
  . = ALIGN(8);
  _bss_begin = ALIGN(8); 
  .bss (NOLOAD): { *(.bss) }
  _bss_end = .;
  _stack_top = 0x80000 + 0x8000; /* 32 KB stack */
}
