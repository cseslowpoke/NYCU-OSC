#include "fs/tmpfs.h"
#include "fs/vfs.h"
#include "common/string.h"
#include "mm/slab.h"

int tmpfs_lookup(struct vnode* dir_node, struct vnode** target, const char* component_name);
int tmpfs_create(struct vnode* dir_node, struct vnode** target, const char* component_name);
int tmpfs_mkdir(struct vnode* dir_node, struct vnode** target, const char* component_name);


struct vnode_operations tmpfs_v_ops = {
    .lookup = tmpfs_lookup,
    .create = tmpfs_create,
    .mkdir = tmpfs_mkdir
};

int tmpfs_write(struct file* file, const void* buf, uint32_t len);
int tmpfs_read(struct file* file, void* buf, uint32_t len);

struct file_operations tmpfs_f_ops = {
    .write = tmpfs_write,
    .read = tmpfs_read,
    .open = NULL, //
    .close = NULL // 
};


int tmpfs_lookup(struct vnode* dir_node, struct vnode** target, const char* component_name) {
    struct tmpfs_internal* internal = (struct tmpfs_internal*)dir_node->internal;
    if (internal->type != TMPFS_DIRECTORY) {
        return -1; // Not a directory
    }

    for (int i = 0; i < internal->content.dir.num_entries; ++i) {
        struct vnode* child_vnode = internal->content.dir.entries[i];
        struct tmpfs_internal* child_internal = (struct tmpfs_internal*)child_vnode->internal;
        
        if (strcmp(child_internal->name, component_name) == 0) {
            *target = child_vnode;
            return 0; // Found
        }
    }

    *target = NULL;
    return -1; // Not found
}

int tmpfs_create(struct vnode* dir_node, struct vnode** target, const char* component_name) {
    struct tmpfs_internal* parent_internal = (struct tmpfs_internal*)dir_node->internal;
    if (parent_internal->type != TMPFS_DIRECTORY || parent_internal->content.dir.num_entries >= MAX_DIR_ENTRIES) {
        return -1; // Error
    }
    
    if (tmpfs_lookup(dir_node, target, component_name) == 0) {
        return -1; // File already exists
    }

    // 2. create a new internal structure
    struct tmpfs_internal* new_internal = (struct tmpfs_internal*)kmalloc(sizeof(struct tmpfs_internal));
    memset(new_internal, 0, sizeof(struct tmpfs_internal)); // 清空結構
    strcpy(new_internal->name, component_name);
    new_internal->name[MAX_FILE_NAME] = '\0';
    new_internal->type = TMPFS_FILE;
    new_internal->content.file.size = 0;

    // 3. create a new vnode 
    struct vnode* new_vnode = (struct vnode*)kmalloc(sizeof(struct vnode));
    memset(new_vnode, 0, sizeof(struct vnode));
    new_vnode->mount = dir_node->mount;
    new_vnode->internal = new_internal;
    new_vnode->v_ops = dir_node->v_ops; 
    new_vnode->f_ops = dir_node->f_ops;
    new_vnode->parent = dir_node;

    parent_internal->content.dir.entries[parent_internal->content.dir.num_entries++] = new_vnode;
    
    *target = new_vnode;
    return 0; // Success
}

int tmpfs_mkdir(struct vnode* dir_node, struct vnode** target, const char* component_name) {
    struct tmpfs_internal* parent_internal = (struct tmpfs_internal*)dir_node->internal;
    if (parent_internal->type != TMPFS_DIRECTORY || parent_internal->content.dir.num_entries >= MAX_DIR_ENTRIES) {
        return -1;
    }
    if (tmpfs_lookup(dir_node, target, component_name) == 0) {
        return -1;
    }

    struct tmpfs_internal* new_internal = (struct tmpfs_internal*)kmalloc(sizeof(struct tmpfs_internal));
    memset(new_internal, 0, sizeof(struct tmpfs_internal));
    strcpy(new_internal->name, component_name);
    new_internal->name[MAX_FILE_NAME] = '\0';
    new_internal->type = TMPFS_DIRECTORY;
    new_internal->content.dir.num_entries = 0;

    struct vnode* new_vnode = (struct vnode*)kmalloc(sizeof(struct vnode));
    memset(new_vnode, 0, sizeof(struct vnode));
    new_vnode->mount = dir_node->mount;
    new_vnode->internal = new_internal;
    new_vnode->v_ops = dir_node->v_ops;
    new_vnode->f_ops = dir_node->f_ops;
    new_vnode->parent = dir_node;

    parent_internal->content.dir.entries[parent_internal->content.dir.num_entries++] = new_vnode;

    *target = new_vnode;
    return 0;
}

int tmpfs_write(struct file* file, const void* buf, uint32_t len) {
    struct tmpfs_internal* internal = (struct tmpfs_internal*)file->vnode->internal;
    
    if (file->f_pos + len > MAX_FILE_SIZE) {
        len = MAX_FILE_SIZE - file->f_pos;
    }
    
    memcpy(internal->content.file.data + file->f_pos, buf, len);  
    file->f_pos += len;
    if (file->f_pos > internal->content.file.size) {
        internal->content.file.size = file->f_pos;
    }

    return len;
}

int tmpfs_read(struct file* file, void* buf, uint32_t len) {
    struct tmpfs_internal* internal = (struct tmpfs_internal*)file->vnode->internal;
    uint32_t readable_len = internal->content.file.size - file->f_pos;
    if (len > readable_len) {
        len = readable_len;
    }

    if (len == 0) return 0; // EOF

    memcpy(buf, internal->content.file.data + file->f_pos, len);
    file->f_pos += len;

    return len;
}

int tmpfs_setup_mount(struct filesystem* fs, struct mount* mount) {
    // 1. construct root vnode and internal structure
    struct vnode* root_vnode = (struct vnode*)kmalloc(sizeof(struct vnode));
    struct tmpfs_internal* root_internal = (struct tmpfs_internal*)kmalloc(sizeof(struct tmpfs_internal));

    // 2. initialize root internal structure
    strcpy(root_internal->name, "/");
    root_internal->type = TMPFS_DIRECTORY;
    root_internal->content.dir.num_entries = 0;

    // 3. setup root vnode
    root_vnode->mount = mount;
    root_vnode->v_ops = &tmpfs_v_ops;
    root_vnode->f_ops = &tmpfs_f_ops;
    root_vnode->internal = root_internal;
    root_vnode->parent = root_vnode;

    // 4. setup root vnode in mount structure
    mount->root = root_vnode;
    mount->fs = fs;

    return 0;
}


struct filesystem tmpfs = {
    .name = "tmpfs",
    .setup_mount = tmpfs_setup_mount
};

void tmpfs_init(void) {
    register_filesystem(&tmpfs);
}