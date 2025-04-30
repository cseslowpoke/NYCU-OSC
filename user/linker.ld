SECTIONS
{
  . = 0x0;
  .start : {
    .start
  }
  . = ALIGN(8);
  .text : {
    .text*}
  . = ALIGN(8);
  .rodata : {
    *(.rodata*)
    . = ALIGN(8);
  }
  . = ALIGN(8);
  .data : { *(.data*) }
  . = ALIGN(8);
  .bss : { *(.bss*) }
}

