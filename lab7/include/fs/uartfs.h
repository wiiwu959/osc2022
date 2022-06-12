#ifndef _UARTFS_H
#define _UARTFS_H

#include <fs/vfs.h>
#include <list.h>

#define UART_MASK  0060000
#define UART_DIR   0040000
#define UART_FILE  0000000

#define UART_MAX_FILES   16

extern struct filesystem uartfs;

int uartfs_setup_mount(struct filesystem *fs, struct mount *mount);

// file operations
int uartfs_write(struct file* file, const void* buf, size_t len);
int uartfs_read(struct file* file, void* buf, size_t len);

// vnode operations
int uartfs_lookup(struct vnode *dir_node, struct vnode **target,
                 const char *component_name);
int uartfs_create(struct vnode *dir_node, struct vnode **target,
                 const char *component_name);
int uartfs_mkdir(struct vnode* dir_node, struct vnode** target,
                 const char* component_name);
int uartfs_isdir(struct vnode* node);
int uartfs_getsize(struct vnode* node);



#endif  /* _UARTFS_H */