#include "fs/initramfs.h"
#include "fs/vfs.h"
#include "fs/file.h"
#include "common/string.h"
#include "mm/slab.h"
#include "common/printf.h"

int initramfs_lookup(struct vnode* dir_node, struct vnode** target, const char* component_name);

struct vnode_operations initramfs_v_ops = {
    .lookup = initramfs_lookup,
    .create = NULL, // Not supported
    .mkdir = NULL   // Not supported
};


int initramfs_read(struct file* file, void* buf, uint32_t len);
struct file_operations initramfs_f_ops = {
    .write = NULL, // Not supported
    .read = initramfs_read,
    .open = NULL,  // Not supported
    .close = NULL  // Not supported
};

int initramfs_lookup(struct vnode* dir_node, struct vnode** target, const char* component_name) {
    struct initramfs_internal* internal = (struct initramfs_internal*)dir_node->internal;
    if (internal->type != INITRAMFS_DIRECTORY) {
        return -1; // Not a directory
    }

    for (int i = 0; i < internal->content.dir.num_entries; ++i) {
      struct vnode* child_vnode = internal->content.dir.entries[i];
        struct initramfs_internal* child_internal = internal->content.dir.entries[i]->internal;
        if (strcmp(child_internal->name, component_name) == 0) {
            *target = (struct vnode*)child_vnode; // Cast to vnode
            return 0; // Found
        }
    }

    *target = NULL;
    return -1; // Not found
}

int initramfs_read(struct file* file, void* buf, uint32_t len) {
    struct initramfs_internal* internal = (struct initramfs_internal*)file->vnode->internal;
    if (file->f_pos + len > internal->content.file.size) {
        len = internal->content.file.size - file->f_pos; // Adjust length
    }
    memcpy(buf, internal->content.file.data + file->f_pos, len);
    file->f_pos += len;
    return len;
}

int initramfs_setup_mount(struct filesystem* fs, struct mount* mount) {
    mount->fs = fs;
    mount->root = (struct vnode*)kmalloc(sizeof(struct vnode));
    if (!mount->root) {
        return -1; // Memory allocation failed
    }
    
    struct initramfs_internal* root_internal = (struct initramfs_internal*)kmalloc(sizeof(struct initramfs_internal));
    if (!root_internal) {
        kfree(mount->root);
        return -1; // Memory allocation failed
    }
    
    memset(root_internal, 0, sizeof(struct initramfs_internal));
    strcpy(root_internal->name, "/");
    root_internal->type = INITRAMFS_DIRECTORY;
    root_internal->content.dir.num_entries = 0;

    mount->root->internal = root_internal;
    mount->root->v_ops = &initramfs_v_ops;
    mount->root->f_ops = &initramfs_f_ops;

    struct vnode* vfs1 = root_internal->content.dir.entries[0] = (struct vnode*)kmalloc(sizeof(struct vnode));
    root_internal->content.dir.num_entries = 1;
    


    struct initramfs_internal* vfs1_internal = (struct initramfs_internal*)kmalloc(sizeof(struct initramfs_internal));
    vfs1->internal = vfs1_internal;
    vfs1->v_ops = &initramfs_v_ops;
    vfs1->f_ops = &initramfs_f_ops;
    vfs1->mount = mount;
    vfs1->parent = mount->root;

    memset(vfs1_internal, 0, sizeof(struct initramfs_internal));
    strcpy(vfs1_internal->name, "vfs1.img");
    vfs1_internal->type = INITRAMFS_FILE ;
    vfs1_internal->content.file.size = file_find("vfs1.img", (char**)&vfs1_internal->content.file.data);
    return 0; // Success
}

struct filesystem initramfs_fs = {
    .name = "initramfs",
    .setup_mount = initramfs_setup_mount
};

void initramfs_init(void) {
    register_filesystem(&initramfs_fs);
}