#include <fs/uartfs.h>
#include <mini_uart.h>
#include <allocator.h>
#include <printf.h>

struct file_operations uartfs_fops = {
    .write = uartfs_write,
    .read = uartfs_read
};

struct vnode_operations uartfs_vops = {
    .lookup = uartfs_lookup,
    .create = uartfs_create,
    .mkdir = uartfs_mkdir,
    .isdir = uartfs_isdir,
    .getsize = uartfs_getsize
};

struct filesystem uartfs = {
    .name = "uartfs",
    .setup_mount = &uartfs_setup_mount,
    .list = NULL
};

int uartfs_setup_mount(struct filesystem *fs, struct mount *mount)
{
    struct vnode* root_node = kmalloc(sizeof(struct vnode));
    root_node->internal = NULL;
    root_node->mount = NULL;
    root_node->f_ops = &uartfs_fops;
    root_node->v_ops = &uartfs_vops;

    root_node->parent = mount->root;
    // root_node->mount = mount;
    mount->fs = fs;
    mount->root = root_node;
}

// file operations
int uartfs_write(struct file* file, const void* buf, size_t len)
{
    uart_sendn(buf, len);
    return len;
}

int uartfs_read(struct file* file, void* buf, size_t len)
{
    uart_recvn(buf, len);
    return len;
}

// vnode operations
int uartfs_lookup(struct vnode *dir_node, struct vnode **target,
                 const char *component_name)
{
    printf("[*] uartfs: lookup not allowed.\r\n");
    return -1;
}

int uartfs_create(struct vnode *dir_node, struct vnode **target,
                 const char *component_name)
{
    printf("[*] uartfs: create not allowed.\r\n");
    return -1;
}

int uartfs_mkdir(struct vnode* dir_node, struct vnode** target,
                 const char* component_name)
{
    printf("[*] uartfs: mkdir not allowed.\r\n");
    return -1;
}

int uartfs_isdir(struct vnode* node)
{
    return 0;
}

int uartfs_getsize(struct vnode* node)
{
    printf("[*] uartfs: getsize not allowed.\r\n");
    return -1;
}