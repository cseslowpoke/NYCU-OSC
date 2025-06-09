#ifndef __TMPFS_H
#define __TMPFS_H

#include "common/types.h"

#define MAX_FILE_NAME 256
#define MAX_DIR_ENTRIES 16
#define MAX_FILE_SIZE 4096

typedef enum {
    TMPFS_FILE,
    TMPFS_DIRECTORY
} TMPFS_NODE_TYPE;

struct tmpfs_internal {
    char name[MAX_FILE_NAME + 1];
    TMPFS_NODE_TYPE type;

    union {
        struct {
            char data[MAX_FILE_SIZE];
            uint32_t size;
        } file;

        struct {
            struct vnode* entries[MAX_DIR_ENTRIES];
            int num_entries;
        } dir;
    } content;
};

void tmpfs_init(void);

#endif // __TMPFS_H
