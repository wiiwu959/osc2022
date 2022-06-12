#ifndef _CPIOFS_H
#define _CPIOFS_H

#include <fs/vfs.h>
#include <list.h>

#define CPIO_MASK  0060000
#define CPIO_DIR   0040000
#define CPIO_FILE  0000000

#define CPIO_MAX_FILES   16

struct cpio_content {
    char* name;
    int type;

    // type: DIR, size: total vnode num under dir
    // type: FILE, size: data size
    size_t size;
    size_t capacity;

    char* data;
    struct vnode** child_list;
};

extern struct filesystem cpiofs;

int cpiofs_setup_mount(struct filesystem *fs, struct mount *mount);

// file operations
int cpiofs_write(struct file* file, const void* buf, size_t len);
int cpiofs_read(struct file* file, void* buf, size_t len);

// vnode operations
int cpiofs_lookup(struct vnode *dir_node, struct vnode **target,
                 const char *component_name);
int cpiofs_create(struct vnode *dir_node, struct vnode **target,
                 const char *component_name);
int cpiofs_mkdir(struct vnode* dir_node, struct vnode** target,
                 const char* component_name);
int cpiofs_isdir(struct vnode* node);
int cpiofs_getsize(struct vnode* node);
 

#endif  /* _CPIOFS_H */