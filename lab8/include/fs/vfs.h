#ifndef _VFS_H
#define _VFS_H

#include <stddef.h>
#include <stdint.h>
#include <list.h>

#define O_CREAT   00000100

struct vnode {
    struct mount* mount;
    struct vnode_operations* v_ops;
    struct file_operations* f_ops;
    struct vnode* parent;
    
    void* internal;
};

// file handle
struct file {
    struct vnode* vnode;
    size_t f_pos;  // RW position of this file handle
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
    struct list_head list;
};

struct file_operations {
    int (*write)(struct file* file, const void* buf, size_t len);
    int (*read)(struct file* file, void* buf, size_t len);
    int (*sync)();
    // long lseek64(struct file* file, long offset, int whence);
};

struct vnode_operations {
    int (*lookup)(struct vnode* dir_node, struct vnode** target,
                    const char* component_name);
    int (*create)(struct vnode* dir_node, struct vnode** target,
                    const char* component_name);
    int (*mkdir)(struct vnode* dir_node, struct vnode** target,
                const char* component_name);
    int (*isdir)(struct vnode* node);
    int (*getsize)(struct vnode* node);
};

struct mount *rootfs;
int vfs_mount_rootfs(const char* filesystem);

struct list_head registered_list;

void vfs_init(void);
int register_filesystem(struct filesystem* fs);
char* get_component_name(char** pathname);

// file operation
int vfs_open(const char* pathname, int flags, struct file** target);
int vfs_close(struct file* file);
int vfs_write(struct file* file, const void* buf, size_t len);
int vfs_read(struct file* file, void* buf, size_t len);
int vfs_sync(void);

// vnode operation
int vfs_mkdir(const char* pathname);
int vfs_mount(const char *target, const char *filesystem);
int vfs_lookup(const char* pathname, struct vnode** target);
int vfs_isdir(struct vnode* node);

// syscall
// syscall number : 11
int open(const char *pathname, int flags);
// syscall number : 12
int close(int fd);
// syscall number : 13
// remember to return read size or error code
long write(int fd, const void *buf, unsigned long count);
// syscall number : 14
// remember to return read size or error code
long read(int fd, void *buf, unsigned long count);
// syscall number : 15
// you can ignore mode, since there is no access control
int mkdir(const char *pathname, unsigned mode);
// syscall number : 16
// you can ignore arguments other than target (where to mount) and filesystem (fs name)
int mount(const char *src, const char *target, const char *filesystem, unsigned long flags, const void *data);
// syscall number : 17
int chdir(const char *path);
// syscall number : 20
void sync(void);

#endif  /* _VFS_H */