#include <fs/vfs.h>
#include <list.h>
#include <allocator.h>
#include <string.h>
#include <printf.h>
#include <mm.h>
#include <sched.h>

#include <fs/tmpfs.h>
#include <fs/cpiofs.h>
#include <fs/uartfs.h>

int register_filesystem(struct filesystem* fs)
{
    // register the file system to the kernel.
    // you can also initialize memory pool of the file system here.
    
    // check if already registered
    struct filesystem* tmp;
    list_for_each_entry(tmp, &registered_list, list) {
        if(strcmp(tmp->name, fs->name) == 0) {
            return 1;
        }
    }

    // if not registered, add it to registered list
    list_add(&fs->list, &registered_list);
    return 0;
}

// get the first component name in a path
char* get_component_name(char** pathname)
{
    char* path = *pathname;

    if (path[0] == '/') { 
        path++;
        *pathname += 1;
    }
    
    int len = 0;
    while (path[len] != '\0' && path[len] != '/') {
        len++;
    }

    char* target = NULL;
    if (len != 0) {
        target = kmalloc(sizeof(char) * (len + 1));
        memcpy(target, path, len);
        target[len] = '\0';
        
        *pathname += len; // point to next start
    }

    return target;
}

// set target as the vnode of last existed vnode in the path
// set pathname as the component name of last existed vnode

// return 0 if is the last component and existed
// return 1 if is the last component and not existed
// return -1 if dir not existed
int get_component(char** pathname, struct vnode** target)
{
    char* left_pathname = *pathname;
    struct vnode* cwd = NULL;
    if (*left_pathname == '/') {
        cwd = rootfs->root;
    } else {
        cwd = current->cwd;
    }

    char* component_name = get_component_name(&left_pathname);
    struct vnode* child_target = NULL;

    while (component_name) {
        if (!strcmp(component_name, ".")) {
            child_target = cwd;
        } else if (!strcmp(component_name, "..")) {
            if (cwd->parent == NULL) {
                printf("[*] vfs: parent not found.\r\n");
                return -1;
            }
            cwd = cwd->parent;

            // if is mount, then real parent should be mount parent
            if (cwd->mount != NULL) {
                if (cwd->parent == NULL) {
                    printf("[*] vfs: parent not found.\r\n");
                    return -1;
                }
                cwd = cwd->parent;
            }
        } else {
            cwd->v_ops->lookup(cwd, &child_target, component_name);
            if (child_target == NULL) {
                *pathname = component_name;
                *target = cwd;
                if (*left_pathname == '\0') { return 1; }
                else { return -1; }
            }
            
            // if it is mount, go through the mount
            if (child_target->mount != NULL) {
                child_target = child_target->mount->root;
            }

            cwd = child_target;
        }
        
        *pathname = component_name;
        component_name = get_component_name(&left_pathname);
    }
    *target = cwd;
    return 0;
}

void vfs_init()
{
    INIT_LIST_HEAD(&registered_list);
    register_filesystem(&tmpfs);
    vfs_mount_rootfs("tmpfs");

    register_filesystem(&cpiofs);
    vfs_mkdir("/initramfs");
    vfs_mount("/initramfs", "cpiofs");

    register_filesystem(&uartfs);
    vfs_mkdir("/dev");
    vfs_mkdir("/dev/uart");
    vfs_mount("/dev/uart", "uartfs");
}

// mount filesystem as rootfs
int vfs_mount_rootfs(const char* filesystem)
{
    // check if registered, and get filesystem
    int if_registered = 0;
    struct filesystem *tmp;
    list_for_each_entry(tmp, &registered_list, list) {
        if (strcmp(tmp->name, filesystem) == 0) {
            if_registered = 1;
            break;
        }
    }

    if (if_registered == 0) {
        printf("[*] The filesystem is not registered yet.\r\n");
        return -1;
    }

    // initialize rootfs
    rootfs = kmalloc(sizeof(struct mount));
    rootfs->fs = NULL;
    rootfs->root = NULL;

    // if registered, setup mount tmpfs
    tmp->setup_mount(tmp, rootfs);
    return 0;
}

int vfs_open(const char* pathname, int flags, struct file** target)
{
    // 1. Lookup pathname
    char* component_name = pathname;
    struct vnode* child_target;
    int ret = get_component(&component_name, &child_target);

    if (ret == -1) {
        printf("[*] vfs: sth went wront in vfs_open.\r\n");
        return -1;
    }

    // 2. Create a new file handle for this vnode if found.
    if (child_target != NULL && ret == 0) {
        struct file* target_file = kmalloc(sizeof(struct file));
        target_file->vnode = child_target;
        target_file->f_ops = child_target->f_ops;
        target_file->f_pos = 0;
        target_file->flags = 0;
        *target = target_file;
    }

    // 3. Create a new file if O_CREAT is specified in flags and vnode not found
    else {
        if ((flags & O_CREAT) && (ret == 1)) {
            struct vnode* cwd = child_target;
            cwd->v_ops->create(cwd, &child_target, component_name);
            struct file* target_file = kmalloc(sizeof(struct file));
            target_file->vnode = child_target;
            target_file->f_ops = child_target->f_ops;
            target_file->f_pos = 0;
            target_file->flags = flags;
            *target = target_file;
        } else {
            printf("[*] vfs: file %s not existed.\r\n", pathname);
            return -1;
        }
    }
    
    // lookup error code shows if file exist or not or other error occurs
    // 4. Return error code if fails
    return 0;
}

int vfs_close(struct file* file)
{
    // 1. release the file handle
    // 2. Return error code if fails
    kfree(file);
    return 0;
}

int vfs_write(struct file* file, const void* buf, size_t len)
{
    // 1. write len byte from buf to the opened file.
    // 2. return written size or error code if an error occurs.
    if (len < 0) { return -1; }
    return file->f_ops->write(file, buf, len);
    
}

int vfs_read(struct file* file, void* buf, size_t len)
{
    // 1. read min(len, readable size) byte to buf from the opened file.
    // 2. block if nothing to read for FIFO type
    // 2. return read size or error code if an error occurs.
    return file->f_ops->read(file, buf, len);
}

// return -1 if mkdir failed
int vfs_mkdir(const char* pathname)
{
    struct vnode* cwd;
    struct vnode* child_target = NULL;
    char* component_name = pathname;
    int ret = get_component(&component_name, &child_target);
    if (ret == 1) {
        cwd = child_target;
        cwd->v_ops->mkdir(cwd, &child_target, component_name);
        return 0;
    } else if (ret == 0) {
        printf("[*] vfs: dir already existed.\r\n");
    }
    return -1;
}

// target is where to mount
int vfs_mount(const char *target, const char *filesystem)
{
    // check if registered and get filesystem
    int if_registered = 0;
    struct filesystem *tmp;
    list_for_each_entry(tmp, &registered_list, list) {
        if (!strcmp(tmp->name, filesystem)) {
            if_registered = 1;
            break;
        }
    }

    if (if_registered == 0) {
        printf("[*] The filesystem is not registered yet.\r\n");
        return -1;
    }

    // find the vnode to mount
    struct vnode* to_mount;
    int ret = vfs_lookup(target, &to_mount);
    if (ret != 0) {
        printf("[*] Mount filesystem %s to %s failed.\r\n", filesystem, target);
        return ret;
    }
    
    struct mount *mount = kmalloc(sizeof(struct mount));
    mount->fs = tmp;
    mount->root = to_mount;
    to_mount->mount = mount;

    return tmp->setup_mount(tmp, mount);
}

// find the vnode and assigned to target
// return -1 if the path not found
// return 1 if the last file or dir not found
int vfs_lookup(const char* pathname, struct vnode** target)
{
    struct vnode* child_target = NULL;
    char* component_name = pathname;
    int ret = get_component(&component_name, &child_target);
    *target = child_target;
    return ret;
}

int vfs_isdir(struct vnode* node)
{
    return node->v_ops->isdir(node);
}

// syscall
// syscall number : 11
int open(const char *pathname, int flags)
{
    int fd = ++current->fd_num;
    if (fd >= 16) {
        printf("[*] syscall open: fd over 16.\r\n");
        return -1;
    }

    int ret = vfs_open(pathname, flags, &current->fd_table[fd]);
    if (ret < 0) { return ret; }
    else { return fd; }
}

// syscall number : 12
int close(int fd)
{
    if (fd < 0) {
        printf("[*] syscall close: fd less than 0.\r\n");
        return -1;
    }

    if (fd >= 16) {
        printf("[*] syscall close: fd over 16.\r\n");
        return -1;
    }

    if (current->fd_table[fd] == NULL) {
        printf("[*] syscall close: file not opened.\r\n");
        return -1;
    }

    int ret = vfs_close(current->fd_table[fd]);
    if (ret < 0) { return ret; }
    else { return fd; }
}

// syscall number : 13
// remember to return read size or error code
long write(int fd, const void *buf, unsigned long count)
{
    if (fd < 0) {
        printf("[*] syscall write: fd less than 0.\r\n");
        return -1;
    }

    if (fd >= 16) {
        printf("[*] syscall write: fd over 16.\r\n");
        return -1;
    }

    if (current->fd_table[fd] == NULL) {
        printf("[*] syscall write: file not opened.\r\n");
        return -1;
    }

    long ret = vfs_write(current->fd_table[fd], buf, count);
    return ret;
}

// syscall number : 14
// remember to return read size or error code
long read(int fd, void *buf, unsigned long count)
{
    if (fd < 0) {
        printf("[*] syscall read: fd less than 0.\r\n");
        return -1;
    }

    if (fd >= 16) {
        printf("[*] syscall read: fd over 16.\r\n");
        return -1;
    }

    if (current->fd_table[fd] == NULL) {
        printf("[*] syscall read: file not opened.\r\n");
        return -1;
    }

    return vfs_read(current->fd_table[fd], buf, count);
}

// syscall number : 15
// you can ignore mode, since there is no access control
int mkdir(const char *pathname, unsigned mode)
{
    return vfs_mkdir(pathname);
}

// syscall number : 16
// you can ignore arguments other than target (where to mount) and filesystem (fs name)
int mount(const char *src, const char *target, const char *filesystem, unsigned long flags, const void *data)
{
    return vfs_mount(target, filesystem);
}

// syscall number : 17
int chdir(const char *path)
{
    struct vnode *target;
    int ret = vfs_lookup(path, &target);

    if (ret < 0) {
        return ret;
    }

    if (vfs_isdir(target) != 1) {
        printf("[*] syscall chdir: not a dir.\r\n");
        return -1;
    }

    current->cwd = target;
    return ret;
}