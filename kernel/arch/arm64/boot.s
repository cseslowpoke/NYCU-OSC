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
  ldr x0, =(0 << 14) | (2 << 30) | ((64 - 48) << 0) | ((64 - 48) << 16)
  msr tcr_el1, x0
  ldr x0, =(0 << 0) | (0x44 << 8)
  msr mair_el1, x0

  mov x0, 0x1000
  mov x1, 0x2000
  ldr x2, =3
  orr x2, x1, x2
  str x2, [x0]
  ldr x2, =(1 << 10) | (0 << 2) | 1
  mov x3, 0x00000000
  orr x3, x2, x3
  str x3, [x1]
  mov x3, 0x40000000
  orr x3, x2, x3
  str x3, [x1, 8]
  msr ttbr0_el1, x0
  msr ttbr1_el1, x0

  mrs x2, sctlr_el1
  orr x2, x2, 1
  msr sctlr_el1, x2
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
  ldr x1, = main
  br x1
1:
  b 1b  

from_el2_to_el1:
  mov x0, (1 << 31) // EL1 uses aarch64
  msr hcr_el2, x0
  mov x0, 0x3c5
  msr spsr_el2, x0
  msr elr_el2, lr
  eret // return to EL1
