#ifndef _TMPFS_H
#define _TMPFS_H

#include <fs/vfs.h>

#define TMP_FILE    0
#define TMP_DIR     1

#define TMP_MAX_FILES   16

struct content {
    char* name;
    int type;
    
    // type: DIR, size: total vnode num under dir
    // type: FILE, size: data size
    size_t size;
    size_t capacity;

    char* data;
    struct vnode** child_list;
};

extern struct filesystem tmpfs;

int tmpfs_init();
int tmpfs_setup_mount(struct filesystem *fs, struct mount *mount);

struct vnode* new_vnode();
void build_root(struct vnode *root_dir);

// file operations
int tmpfs_write(struct file* file, const void* buf, size_t len);
int tmpfs_read(struct file* file, void* buf, size_t len);

// vnode operations
int tmpfs_lookup(struct vnode *dir_node, struct vnode **target,
                 const char *component_name);
int tmpfs_create(struct vnode *dir_node, struct vnode **target,
                 const char *component_name);
int tmpfs_mkdir(struct vnode* dir_node, struct vnode** target,
                 const char* component_name);
int tmpfs_isdir(struct vnode* node);
int tmpfs_getsize(struct vnode* node);

#endif  /* _TMPFS_H */