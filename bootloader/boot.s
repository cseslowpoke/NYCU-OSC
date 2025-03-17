.section ".text.relo"
.globl _start
_start:
  mov x4, x0
  ldr x0, =_boot_begin
  ldr x1, =0x40000
  ldr x2, =_bootloader_size

relocation_loop:
  ldr x3, [x0], #8
  str x3, [x1], #8
  sub x2, x2, #8
  cbnz x2, relocation_loop
  
go_new:
  ldr x0, =0x40000
  br x0

.section ".text.boot"
.globl _start_boot

_start_boot:
  mrs x0, CurrentEL
  lsr x0, x0, #2
  cmp x0, #2
  bne 1f
  bl from_el2_to_el1

1:
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

from_el2_to_el1:
  mov x0, (1 << 31) // EL1 uses aarch64
  msr hcr_el2, x0
  mov x0, 0x3c5 // EL1h (SPSel = 1) with interrupt disabled
  msr spsr_el2, x0
  msr elr_el2, lr
  eret // return to EL1
