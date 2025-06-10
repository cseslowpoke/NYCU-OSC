#ifndef __TASK_H
#define __TASK_H

#include "common/list.h"
#include "core/signal.h"
#include "fs/vfs.h"

#define KERNEL_STACK_SIZE 4096
#define USER_STACK_SIZE 16384

enum task_state {
  TASK_RUNNING,
  TASK_SLEEPING,
  TASK_ZOMBIE,
};

enum task_type {
  TASK_KERNEL,
  TASK_USER,
};

typedef struct trapframe {
  uint64_t elr_el1;  // Exception Link Register
  uint64_t spsr_el1; // Saved Program Status Register
  uint64_t sp_el0;   // Stack Pointer for EL0
  uint64_t gpr[31];  // General Purpose Registers
} trapframe_t;

typedef struct context {
  uint64_t x19;
  uint64_t x20;
  uint64_t x21;
  uint64_t x22;
  uint64_t x23;
  uint64_t x24;
  uint64_t x25;
  uint64_t x26;
  uint64_t x27;
  uint64_t x28;
  uint64_t fp; // x29
  uint64_t lr; // x30
  uint64_t sp;
} context_t;

typedef struct task_struct {
  // kernel space
  context_t context;
  int pid;
  enum task_state state;
  enum task_type type;
  void *stack;
  list_head_t task_list;
  void (*fn)(void);
  uint32_t irq_priority;

  // user space
  void *user_stack;
  trapframe_t *trapframe; // For user tasks
  void *prog;
  uint64_t prog_size;
  signal_info_t signal;

  // virtual memory support
  uint64_t *pgd;            // Page Global Directory
  list_head_t vm_area_list; // List of virtual memory areas

  // vfs support
  struct file *file_table[16]; // File descriptors
  struct vnode *cwd;          // Current working directory
} task_struct_t;

task_struct_t *task_create_kernel(void (*fn)(void));

task_struct_t *task_create_user();

void task_exit(task_struct_t *task);

void task_destory(task_struct_t *task);

#define get_current()                                                          \
  ({                                                                           \
    uint64_t tpidr_el1;                                                        \
    __asm__ volatile("mrs %0, tpidr_el1" : "=r"(tpidr_el1));                   \
    (task_struct_t *)tpidr_el1;                                                \
  })

void task_set_current(task_struct_t *task);

void task_entry_wrapper();

void task_return_el0();

void task_test();

#endif // __TASK_H
