#include "fs/vfs.h"
#include "fs/tmpfs.h"
#include "common/printf.h"
#include "mm/slab.h"
#include "common/string.h"

// helper function declaration
int get_parent_path(const char* full_path, char* parent_path, char* component_name);
struct vnode* check_mount_point(struct vnode* node);


struct filesystem* registered_fs[10];
static int num_fs = 0;

struct mount* rootfs;

struct mount_info {
  struct vnode* mount_point; 
  struct mount* mount;
};
static struct mount_info mount_table[10];
static int mount_count = 0;

int register_filesystem(struct filesystem* fs) {
    if (num_fs < 10) {
        registered_fs[num_fs++] = fs;
        printf("Registered filesystem: %s\n", fs->name);
        return 0;
    }
    return -1; // No space
}

void vfs_init() {
    tmpfs_init();
    struct mount* mount = (struct mount*)kmalloc(sizeof(struct mount));
    registered_fs[0]->setup_mount(registered_fs[0], mount);
    rootfs = mount; // Set the global rootfs
    // cwd = rootfs->root; // Set current working directory to root
    printf("VFS initialized with rootfs: %s\n", rootfs->fs->name);
}

int vfs_lookup(const char* pathname, struct vnode** target) {
    struct vnode* current_vnode = rootfs->root;
    char* path = strdup(pathname);
    char* component = strtok(path, '/');

    while (component != NULL) {
        struct vnode* next_vnode = NULL;
        if (current_vnode->v_ops->lookup(current_vnode, &next_vnode, component) != 0) {
            kfree(path);
            return -1;
        }
        current_vnode = check_mount_point(next_vnode);
        
        component = strtok(NULL, '/');
    }

    *target = current_vnode;
    kfree(path);
    return 0; // 成功
}

#define O_CREAT 64
int vfs_open(const char* pathname, int flags, struct file** target) {
    struct vnode* node;
    int result = vfs_lookup(pathname, &node);

    if (result != 0) { 
        if (flags & O_CREAT) { // if O_CREAT is set, we need to create the file

            // 1. find parent path and component name
            // e.g., "/a/b/c" -> parent_path="/a/b", component_name="c"
            char parent_path[256], componenet[256];
            if (get_parent_path(pathname, parent_path, componenet) != 0) {
                return -1;
            }

            struct vnode* parent_node;
            if (vfs_lookup(parent_path, &parent_node) != 0) return -1;

            // 2. create the vnode
            if (parent_node->v_ops->create(parent_node, &node, componenet) != 0) {
                return -1;
            }
        } else {
            return -1;
        }
    }

    // 3. file handle
    struct file* file_handle = (struct file*)kmalloc(sizeof(struct file));
    file_handle->vnode = node;
    file_handle->f_pos = 0;
    file_handle->f_ops = node->f_ops;
    file_handle->flags = flags;
    
    if (file_handle->f_ops->open) {
        file_handle->f_ops->open(node, &file_handle);
    }

    *target = file_handle;
    return 0; // Success
}


int vfs_write(struct file* file, const void* buf, uint32_t len) {
    if (!file || !file->f_ops->write) return -1;
    return file->f_ops->write(file, buf, len);
}

int vfs_read(struct file* file, void* buf, uint32_t len) {
    if (!file || !file->f_ops->read) return -1;
    return file->f_ops->read(file, buf, len);
}

int vfs_close(struct file* file) {
    if (!file) return -1;
    if (file->f_ops->close) {
        file->f_ops->close(file);
    }
    kfree(file);
    return 0;
}

int vfs_mkdir(const char* pathname) {
    // 1. find parent path and component name
    // e.g., "/a/b/c" -> parent_path="/a/b", component_name="c"
    char parent_path[256], componenet[256];
    if (get_parent_path(pathname, parent_path, componenet) != 0) {
        return -1;
    }

    // 2. find parent vnode
    struct vnode* parent_node;
    if (vfs_lookup(parent_path, &parent_node) != 0) {
        return -1;
    }
    
    // 3. call mkdir operation
    struct vnode *tmp_node = kmalloc(sizeof(struct vnode));
    if (parent_node->v_ops->mkdir(parent_node, &tmp_node, componenet) != 0) {
        return -1;
    }

    return 0;
}


int vfs_mount(const char *src, const char *target, const char *filesystem, unsigned long flags, const void *data) {
  if (mount_count >= 10) {
    return -1; // No space for more mounts
  }

  // 1. find the filesystem
  for (int i = 0; i < num_fs; i++) {
    if (strcmp(registered_fs[i]->name, filesystem) == 0) {
      struct vnode *target_vnode;
      // 2. find the target vnode
      if (vfs_lookup(target, &target_vnode) != 0) {
        return -1; // Target path not found
      }
      struct mount *new_mount = (struct mount *)kmalloc(sizeof(struct mount));
      if (new_mount == NULL) {
        return -1; // Memory allocation failed
      }
      registered_fs[i]->setup_mount(registered_fs[i], new_mount);
    
      mount_table[mount_count].mount_point = target_vnode;
      mount_table[mount_count].mount = new_mount;
      mount_count++;
      printf("Mounted %s at %s using filesystem %s\n", src, target, filesystem);
      return 0;
    }
  }
  return -1; // Filesystem not found
}

// Helper function 

int get_parent_path(const char* full_path, char* parent_path, char* component_name) {
    if (!full_path || !parent_path || !component_name) {
        return -1; 
    }

    uint32_t len = strlen(full_path);
    if (len == 0 || len > 255) {
        return -1;
    }
    
    char path_copy[256];
    strcpy(path_copy, full_path);

    if (len > 1 && path_copy[len - 1] == '/') {
        path_copy[len - 1] = '\0';
    }

    if (strcmp(path_copy, "/") == 0) {
        return -1;
    }

    char* last_slash = strrchr(path_copy, '/');

    if (last_slash == NULL) {
        return -1;
    }

    strcpy(component_name, last_slash + 1);

    if (last_slash == path_copy) {
        strcpy(parent_path, "/");
    } else {
        *last_slash = '\0';
        strcpy(parent_path, path_copy);
    }
    
    return 0;
}

struct vnode* check_mount_point(struct vnode* node) {
    for (int i = 0; i < mount_count; i++) {
        if (mount_table[i].mount_point == node) {
            // if the node is a mount point, return the root of the mounted filesystem
            return mount_table[i].mount->root;
        }
    }
    // if the node is not a mount point, return the node itself
    return node;
}