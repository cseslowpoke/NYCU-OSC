#ifndef __INITRAMFS_H
#define __INITRAMFS_H
#include "common/types.h"
#include "fs/vfs.h"

#define MAX_FILE_NAME 256
#define MAX_DIR_ENTRIES 16
#define MAX_FILE_SIZE 409600

typedef enum {
    INITRAMFS_FILE,
    INITRAMFS_DIRECTORY
} INITRAMFS_NODE_TYPE;


struct initramfs_internal {
  char name[MAX_FILE_NAME + 1];
  INITRAMFS_NODE_TYPE type;

  union {
    struct {
      void *data;
      uint32_t size;
    } file;

    struct {
      struct vnode *entries[MAX_DIR_ENTRIES];
      int num_entries;
    } dir;
  } content;
};

void initramfs_init(void);

#endif // __INITRAMFS_H