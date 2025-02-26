SECTIONS
{
  . = 0x80000;
  .text.boot : { KEEP(*(.text.boot)) }
  .text : { *(.text) }

  .rodata : { *(.rodata) }
  .data : { *(.data) }

  /* record start/end of bss section to fill 0 */
  . = ALIGN(8);
  bss_begin = .;
  .bss (NOLOAD): { *(.bss) }
  bss_end = .;

  _stack_top = 0x80000 + 0x8000; /* 32 KB stack */
  _end = .;
}
