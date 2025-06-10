#include "core/syscall.h"
#include "common/printf.h"
#include "common/types.h"
#include "core/exec.h"
#include "core/fork.h"
#include "core/sched.h"
#include "core/signal.h"
#include "core/task.h"
#include "drivers/irq.h"
#include "drivers/mailbox.h"
#include "drivers/uart.h"
#include "mm/mm.h"
#include "mm/mmu.h"
#include "mm/slab.h"

typedef uint64_t (*syscall_handler_t)(trapframe_t *tf);
#define SYSCALL_DEF(num, name) [num] = (void *)sys_##name,

static syscall_handler_t syscall_table[] = {
#include "core/syscall_table.h"
};

#undef SYSCALL_DEF

int32_t syscall_handler(trapframe_t *tf) {
  uint64_t syscall_num = tf->gpr[8]; // x8
  if (syscall_num > SYSCALL_MAX) {
    return -1;
  }
  syscall_handler_t syscall_fn = syscall_table[syscall_num];
  if (syscall_fn == NULL) {
    return -1;
  }
  syscall_fn(tf);
  return 0;
}

void sys_getpid(trapframe_t *tf) {
  task_struct_t *current = get_current();
  tf->gpr[0] = current->pid;
}

void sys_uart_read(trapframe_t *tf) {
  char *buf = (char *)tf->gpr[0];
  uint32_t size = (uint32_t)tf->gpr[1];
  ENABLE_IRQ();
  for (int i = 0; i < size; i++) {
    buf[i] = uart_recv();
  }
  DISABLE_IRQ();
  tf->gpr[0] = size;
}

void sys_uart_write(trapframe_t *tf) {
  const char *buf = (const char *)tf->gpr[0];
  uint32_t size = (uint32_t)tf->gpr[1];
  ENABLE_IRQ();
  for (int i = 0; i < size; i++) {
    uart_send(buf[i]);
  }
  DISABLE_IRQ();
  tf->gpr[0] = size;
}

void sys_exec(trapframe_t *tf) {
  const char *name = (const char *)tf->gpr[0];
  char *const *argv = (char *const *)tf->gpr[1];

  tf->gpr[0] = do_exec(name, argv);
}

void sys_fork(trapframe_t *tf) { tf->gpr[0] = do_fork(); }

void sys_exit(trapframe_t *tf) {
  task_struct_t *current = get_current();
  current->state = TASK_ZOMBIE;
  while (1)
    ;
}

void sys_mbox_call(trapframe_t *tf) {
  uint8_t ch = tf->gpr[0];
  uint32_t *mbox = (uint32_t *)tf->gpr[1];

  tf->gpr[0] = mailbox_call_with_mail(ch, mbox);
}

void sys_pkill(trapframe_t *tf) {
  uint32_t pid = tf->gpr[0];
  task_struct_t *current = get_current();
  if (pid == current->pid) {
    current->state = TASK_ZOMBIE;
    sched();
  } else {
    sched_kill_task(pid);
  }
}

void sys_signal(trapframe_t *tf) {
  uint32_t signum = tf->gpr[0];
  sig_handler_t handler = (sig_handler_t)tf->gpr[1];
  signal_register(signum, handler);
}

void sys_kill(trapframe_t *tf) {
  uint32_t pid = tf->gpr[0];
  uint32_t signum = tf->gpr[1];
  signal_send(pid, signum);
}

#define PROT_NONE 0
#define PROT_READ 1
#define PROT_WRITE 2
#define PROT_EXEC 4

#define MAP_ANONYMOUS 0x20
#define MAP_POPULATE 0x8000

void sys_mmap(trapframe_t *tf) {
  uint64_t addr = tf->gpr[0];
  uint64_t size = tf->gpr[1];
  uint64_t prot = tf->gpr[2];
  uint64_t flag = tf->gpr[3];
  uint64_t fd = tf->gpr[4];
  uint64_t offset = tf->gpr[5];

  task_struct_t *current = get_current();
  // TODO: implement
  addr = vma_find_unmapped_area_near(current, addr, size);
  vm_area_t *vma = kmalloc(sizeof(vm_area_t));
  INIT_LIST_HEAD(&vma->list);
  vma->start = addr;
  vma->end = round_up(addr + size, PAGE_SIZE);
  // set prtotection
  vma->prot = MAIR_NORMAL;
  if (prot & PROT_WRITE) {
    vma->prot |= AP_RW_EL0;
  } else if (prot & PROT_READ) {
    vma->prot |= AP_RO_EL0;
  }
  if (!(prot & PROT_EXEC)) {
    vma->prot |= PD_UXN;
  }
  if (prot != 0) {
    vma->prot |= PD_ACCESS;
  }

  // set flag
  vma->flags = 0;
  vma_insert(current, vma);

  if (flag & MAP_POPULATE) {
    void *ptr = kmalloc(size);
    mmu_map(current->pgd, vma->start, (uint64_t)virt_to_phy(ptr), size,
            vma->prot);
  }
  tf->gpr[0] = vma->start;
}

// int open(const char *pathname, int flags);
void sys_open(trapframe_t *tf) {
  const char *pathname = (const char*)tf->gpr[0];
  int flags = tf->gpr[1];
  
  printf("open: %s\r\n", pathname);

  task_struct_t *task = get_current(); 
  int i;
  for (i = 0; i < 16; i++) {
    if (task->file_table[i] == NULL) {
      break;
    }
  }
  if (i == 16) {
    printf("No available file descriptor\r\n");
    return;
  }
  vfs_open(pathname, flags, &task->file_table[i], task->cwd);
  printf("File %s opened with fd %d\r\n", pathname, i);
  tf->gpr[0] = i; // Return the file descriptor
  if (task->file_table[i] == NULL) {
    printf("Failed to open file %s\r\n", pathname);
    tf->gpr[0] = -1; // Indicate failure
    return;
  }
}

// int close(int fd);
void sys_close(trapframe_t *tf) {
  int fd = tf->gpr[0];
  printf("close %d\r\n", fd);
  
  task_struct_t *task = get_current();
  if (fd < 0 || fd >= 16) {
    printf("Invalid file descriptor %d\r\n", fd);
    return;
  }
  struct file *file = task->file_table[fd];
  if (file == NULL) {
    printf("File descriptor %d is not open\r\n", fd);
    return;
  }
  // Call the close operation of the file
  vfs_close(file);
  // Clear the file descriptor
  task->file_table[fd] = NULL;
  printf("File descriptor %d closed\r\n", fd);
}

// long write(int fd, const void *buf, unsigned long count);
void sys_write(trapframe_t *tf) {
  int fd = tf->gpr[0];
  const void* buf = (void*) tf->gpr[1];
  uint64_t count = tf->gpr[2];

  printf("write: %d %p\r\n", fd, buf);
  task_struct_t *task = get_current();
  if (fd < 0 || fd >= 16) {
    printf("Invalid file descriptor %d\r\n", fd);
    return;
  }
  struct file *file = task->file_table[fd];
  if (file == NULL) {
    printf("File descriptor %d is not open\r\n", fd);
    return;
  }
  // Call the write operation of the file
  int ret = vfs_write(file, buf, count);
  if (ret < 0) {
    printf("Failed to write to file descriptor %d\r\n", fd);
    return;
  }
  printf("Wrote %d bytes to file descriptor %d\r\n", ret, fd);
  tf->gpr[0] = ret; // Return the number of bytes written

}

// long read(int fd, void *buf, unsigned long count);
void sys_read(trapframe_t *tf) {
  int fd = tf->gpr[0];
  void* buf = (void*) tf->gpr[1];
  uint64_t count = tf->gpr[2];

  printf("read: %d\r\n",fd);
  task_struct_t *task = get_current();
  if (fd < 0 || fd >= 16) {
    printf("Invalid file descriptor %d\r\n", fd);
    return;
  }
  struct file *file = task->file_table[fd];
  if (file == NULL) {
    printf("File descriptor %d is not open\r\n", fd);
    return;
  }
  // Call the read operation of the file
  int ret = vfs_read(file, buf, count);
  if (ret < 0) {
    printf("Failed to read from file descriptor %d\r\n", fd);
    return;
  }
  printf("Read %d bytes from file descriptor %d\r\n", ret, fd);
  tf->gpr[0] = ret; // Return the number of bytes read
}

// int mkdir(const char *pathname, unsigned mode);
void sys_mkdir(trapframe_t *tf) {
  const char *pathname = (const char*) tf->gpr[0];
  uint32_t mode = tf->gpr[1];
  
  printf("mkdir: %s\r\n",pathname);
  
  task_struct_t *task = get_current();
  int i;
  for (i = 0; i < 16; i++) {
    if (task->file_table[i] == NULL) {
      break;
    }
  }
  if (i == 16) {
    printf("No available file descriptor\r\n");
    return;
  }
  vfs_mkdir(pathname, task->cwd);
  printf("Directory %s created with fd %d\r\n", pathname, i);
  tf->gpr[0] = i; // Return the file descriptor
  if (task->file_table[i] == NULL) {
    printf("Failed to create directory %s\r\n", pathname);
    tf->gpr[0] = -1; // Indicate failure
    return;
  }
}

// int mount(const char *src, const char *target, const char *filesystem, unsigned long flags, const void *data);
void sys_mount(trapframe_t *tf) {
  const char *src = (const char*) tf->gpr[0];
  const char *target = (const char*) tf->gpr[1];
  const char *fs = (const char*) tf->gpr[2];
  uint64_t flags = tf->gpr[3];
  const void* data = (const void*) tf->gpr[4];
  printf("mount: %s %s\r\n", src, target);
  
  task_struct_t *task = get_current();

  vfs_mount(src, target, fs, flags, data, task->cwd);
}

// int chdir(const char *path);
void sys_chdir(trapframe_t *tf) {
  const char *path = (const char*) tf->gpr[0];

  printf("chdir: %s\r\n", path);
  
  task_struct_t *task = get_current();
  if (vfs_chdir(path, &task->cwd) != 0) {
    printf("Failed to change directory to %s\r\n", path);
    tf->gpr[0] = -1; // Indicate failure
  } else {
    printf("Changed directory to %s\r\n", path);
    tf->gpr[0] = 0; // Indicate success
  }
}
 
void sys_sigreturn(trapframe_t *tf) { signal_return(); }
