.section ".text.boot"
.globl _start
_start:
  mov x4, x0
  mrs x0, sctlr_el1
  bic X0, X0, #0x1
  msr sctlr_el1, x0
  isb
  mrs x0, mpidr_el1
  and x0, x0, #0xFF
  cbz x0, clear_bss

non_primary_core:
  wfe
  b non_primary_core

clear_bss:
  ldr x0, = _bss_begin
  ldr x1, = _bss_end
  mov x2, #0

clear_bss_loop:
  cmp x0, x1
  b.ge primary_core
  str x2, [x0], #0x8
  b clear_bss_loop

primary_core:
  ldr x0, = _stack_top
  mov sp, x0
  mov x0, x4
  bl main
1:
  b 1b  
