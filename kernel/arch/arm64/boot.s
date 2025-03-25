.section ".text.boot"
.globl _start
_start:
  mov x4, x0
  mrs x0, CurrentEL
  lsr x0, x0, #2
  cmp x0, #2
  bne 1f
  bl from_el2_to_el1

1:
  // mrs x0, sctlr_el1
  // bic X0, X0, #0x1
  // msr sctlr_el1, x0
  // isb
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
  bl set_exception_vector_table
  ldr x0, = _stack_top
  mov sp, x0
  mov x0, x4
  bl main
1:
  b 1b  

from_el2_to_el1:
  mov x0, (1 << 31) // EL1 uses aarch64
  msr hcr_el2, x0
  mov x0, 0x3c5
  msr spsr_el2, x0
  msr elr_el2, lr
  eret // return to EL1
