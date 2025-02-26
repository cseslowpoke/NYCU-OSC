.section ".text.boot"
.globl _start
_start:
  mrs x0, mpidr_el1
  and x0, x0, #0xFF
  cbz x0, clear_bss

non_primary_core:
  wfe
  b non_primary_core

clear_bss:
  ldr x0, = bss_begin
  ldr x1, = bss_end
  mov x2, #0

clear_bss_loop:
  cmp x0, x1
  b.ge primary_core
  str x2, [x0], #0x8
  b clear_bss_loop

primary_core:
  ldr x0, = _stack_top
  mov sp, x0
  bl main
1:
  b 1b  


