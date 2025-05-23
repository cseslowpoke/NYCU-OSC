.section ".text.exception"
.extern exception_entry
.global exception_vector_table
.global set_exception_vector_table
 
.align 11 // vector table should be aligned to 0x800
exception_vector_table:
  b not_implement_exception// branch to a handler function.
  .align 7 // entry size is 0x80, .align will pad 0
  b not_implement_exception
  .align 7
  b not_implement_exception
  .align 7
  b not_implement_exception
  .align 7

  b _el1_current_el_aarch64_sync
  .align 7
  b _el1_current_el_aarch64_irq
  .align 7
  b not_implement_exception
  .align 7
  b not_implement_exception
  .align 7

  b _el1_lower_el_aarch64_sync
  .align 7
  b _el1_lower_el_aarch64_irq
  .align 7
  b not_implement_exception
  .align 7
  b not_implement_exception
  .align 7

  b not_implement_exception
  .align 7
  b not_implement_exception
  .align 7
  b not_implement_exception
  .align 7
  b not_implement_exception
  .align 7

set_exception_vector_table:
  ldr x0, =exception_vector_table
  msr vbar_el1, x0
  ret

.macro save_all
    sub sp, sp, (3 + 31) * 8
    stp x0, x1, [sp, #(3 * 8) + 16 * 0]
    stp x2, x3, [sp, #(3 * 8) + 16 * 1]
    stp x4, x5, [sp, #(3 * 8) + 16 * 2]
    stp x6, x7, [sp, #(3 * 8) + 16 * 3]
    stp x8, x9, [sp, #(3 * 8) + 16 * 4]
    stp x10, x11, [sp, #(3 * 8) + 16 * 5]
    stp x12, x13, [sp, #(3 * 8) + 16 * 6]
    stp x14, x15, [sp, #(3 * 8) + 16 * 7]
    stp x16, x17, [sp, #(3 * 8) + 16 * 8]
    stp x18, x19, [sp, #(3 * 8) + 16 * 9]
    stp x20, x21, [sp, #(3 * 8) + 16 * 10]
    stp x22, x23, [sp, #(3 * 8) + 16 * 11]
    stp x24, x25, [sp, #(3 * 8) + 16 * 12]
    stp x26, x27, [sp, #(3 * 8) + 16 * 13]
    stp x28, x29, [sp, #(3 * 8) + 16 * 14]
    str x30, [sp, #(3 * 8) + 16 * 15]

    mrs x0, elr_el1
    str x0, [sp, #(0 * 8)]
    mrs x0, spsr_el1
    str x0, [sp, #(1 * 8)]
    mrs x0, sp_el0
    str x0, [sp, #(2 * 8)]
.endm

// load general registers from stack
.macro load_all
    ldr x0, [sp, #(0 * 8)]
    msr elr_el1, x0
    ldr x0, [sp, #(1 * 8)]
    msr spsr_el1, x0
    ldr x0, [sp, #(2 * 8)]
    msr sp_el0, x0

    ldp x0, x1, [sp, #(3 * 8) + 16 * 0]
    ldp x2, x3, [sp, #(3 * 8) + 16 * 1]
    ldp x4, x5, [sp, #(3 * 8) + 16 * 2]
    ldp x6, x7, [sp, #(3 * 8) + 16 * 3]
    ldp x8, x9, [sp, #(3 * 8) + 16 * 4]
    ldp x10, x11, [sp, #(3 * 8) + 16 * 5]
    ldp x12, x13, [sp, #(3 * 8) + 16 * 6]
    ldp x14, x15, [sp, #(3 * 8) + 16 * 7]
    ldp x16, x17, [sp, #(3 * 8) + 16 * 8]
    ldp x18, x19, [sp, #(3 * 8) + 16 * 9]
    ldp x20, x21, [sp, #(3 * 8) + 16 * 10]
    ldp x22, x23, [sp, #(3 * 8) + 16 * 11]
    ldp x24, x25, [sp, #(3 * 8) + 16 * 12]
    ldp x26, x27, [sp, #(3 * 8) + 16 * 13]
    ldp x28, x29, [sp, #(3 * 8) + 16 * 14]
    ldr x30, [sp, #(3 * 8) + 16 * 15]

    add sp, sp, (3 + 31) * 8
.endm


_el1_current_el_aarch64_sync:
  save_all
  bl _el1_current_el_aarch64_sync_handler
  load_all
  eret

_el1_current_el_aarch64_irq:
  msr daifset, #0xf
  save_all
  bl _el1_current_el_aarch64_irq_handler
  load_all
  eret

_el1_lower_el_aarch64_sync:
  ; msr daifclr, #0xf
  save_all
  mov x0, sp
  bl _el1_lower_el_aarch64_sync_handler
  load_all
  eret

_el1_lower_el_aarch64_irq:
  msr daifset, #0xf
  save_all
  mov x0, sp
  bl _el1_lower_el_aarch64_irq_handler
  load_all
  eret

not_implement_exception:
  msr daifset, #0xf
  save_all
  bl default_exception_handler
  load_all
  eret
 
