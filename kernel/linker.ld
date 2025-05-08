SECTIONS
{
  . = 0xffff000000000000;
  . += 0x80000;
  __kernel_start = .;
  .text.boot : { KEEP(*(.text.boot)) }
  .text.exception : {KEEP(*(.text.exception))}
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
  
  . = ALIGN(16); /* stack should aligned 0x10 */
  _stack_top = . + 0x80000; /* 512 KB stack */
  __kernel_end = . + 0x80000;
  /* __simple_alloc_start = 0x10000000; */
  /* __simple_alloc_end =   0x20000000; */
  __simple_alloc_start = 0xffff000000000000 + 0x10000000;
  __simple_alloc_end =   0xffff000000000000 +  0x20000000;
}
