#ifndef __VFS_H
#define __VFS_H

#include "common/types.h"

struct vnode {
  struct mount* mount;
  struct vnode_operations* v_ops;
  struct file_operations* f_ops;
  void* internal;
  struct vnode* parent;
};

// file handle
struct file {
  struct vnode* vnode;
  uint32_t f_pos;  // RW position of this file handle
  struct file_operations* f_ops;
  int flags;
};

struct mount {
  struct vnode* root;
  struct filesystem* fs;
};

struct filesystem {
  const char* name;
  int (*setup_mount)(struct filesystem* fs, struct mount* mount);
};

struct file_operations {
  int (*write)(struct file* file, const void* buf, uint32_t len);
  int (*read)(struct file* file, void* buf, uint32_t len);
  int (*open)(struct vnode* file_node, struct file** target);
  int (*close)(struct file* file);
  // long lseek64(struct file* file, long offset, int whence);
};

struct vnode_operations {
  int (*lookup)(struct vnode* dir_node, struct vnode** target,
                const char* component_name);
  int (*create)(struct vnode* dir_node, struct vnode** target,
                const char* component_name);
  int (*mkdir)(struct vnode* dir_node, struct vnode** target,
              const char* component_name);
};

extern struct mount* rootfs;

int register_filesystem(struct filesystem* fs);

void vfs_init(void);

int vfs_lookup(const char* pathname, struct vnode** target);

int vfs_open(const char* pathname, int flags, struct file** target);

int vfs_write(struct file* file, const void* buf, uint32_t len);

int vfs_read(struct file* file, void* buf, uint32_t len);

int vfs_close(struct file* file);

int vfs_mkdir(const char* pathname);

int vfs_mount(const char* src, const char* target, const char* filesystem,
              unsigned long flags, const void* data);

#endif // __VFS_H
