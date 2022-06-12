#include <fs/cpiofs.h>
#include <cpio.h>
#include <printf.h>
#include <string.h>
#include <allocator.h>
#include <mm.h>

struct file_operations cpiofs_fops = {
    .write = cpiofs_write,
    .read = cpiofs_read
};

struct vnode_operations cpiofs_vops = {
    .lookup = cpiofs_lookup,
    .create = cpiofs_create,
    .mkdir = cpiofs_mkdir,
    .isdir = cpiofs_isdir,
    .getsize = cpiofs_getsize
};

struct filesystem cpiofs = {
    .name = "cpiofs",
    .setup_mount = &cpiofs_setup_mount,
    .list = NULL
};

struct vnode* new_cpio_vnode(char* name, int type)
{
    struct vnode* node = kmalloc(sizeof(struct vnode));

    node->mount = NULL;
    node->f_ops = &cpiofs_fops;
    node->v_ops = &cpiofs_vops;
    node->parent = NULL;

    struct cpio_content* con = kmalloc(sizeof(struct cpio_content));
    con->name = kmalloc(sizeof(char) * (strlen(name) + 1));
    memcpy(con->name, name, strlen(name));
    con->name[strlen(name)] = '\0';
    con->type = type;
    con->size = 0;
    con->capacity = 0;
    con->data = NULL;
    if (type == CPIO_DIR) {
        con->child_list = kmalloc(sizeof(struct vnode*) * CPIO_MAX_FILES);
    } else if (type == CPIO_FILE) {
        con->child_list = NULL;
    }

    node->internal = con;
    return node;
}

int cpiofs_setup_mount(struct filesystem *fs, struct mount *mount)
{
    struct vnode* root_node = new_cpio_vnode("/", CPIO_DIR);
    struct cpio_content* root_con = root_node->internal;

    struct cpio_newc_header *ramfs = (struct cpio_newc_header *)initramfs_loc;
    char* ptr = (char*)ramfs;
    while (1) {
        ramfs = (struct cpio_newc_header *)ptr;
        if (strncmp("070701", ramfs->c_magic, 6)) {
            printf("[*] Not new ASCII format cpio archive.\r\n");
            break;
        }
        
        unsigned int namesize = cpio_read_8hex(ramfs->c_namesize);
        unsigned int filesize = cpio_read_8hex(ramfs->c_filesize);
        int name_align = cpio_align(sizeof(struct cpio_newc_header) + namesize, 4);
        int file_align = cpio_align(filesize, 4);
        
        ptr += sizeof(struct cpio_newc_header);
        char* filename = ptr;
        if (!strcmp("TRAILER!!!", filename)) {
            break;
        } else {
            struct vnode* node = new_cpio_vnode(filename, CPIO_FILE);
            struct cpio_content* con = node->internal;

            ptr += namesize + name_align;
            con->data = kmalloc(filesize);
            con->size = filesize;
            memcpy(con->data, ptr, filesize);

            node->parent = root_node;
            
            root_con->child_list[root_con->size++] = node;
        }
        ptr += filesize + file_align;
    }

    root_node->parent = mount->root;
    mount->fs = fs;
    mount->root = root_node;
}

// file operations
int cpiofs_write(struct file* file, const void* buf, size_t len)
{
    printf("[*] cpiofs: write not allowed.\r\n");
    return -1;
}

int cpiofs_read(struct file* file, void* buf, size_t len)
{
    struct cpio_content* con = file->vnode->internal;
    int max_size = con->size - file->f_pos;

    if (max_size < 0 || con->type != CPIO_FILE) {
        buf = NULL;
        return 0;
    }
    
    int read_size = (max_size > len) ? len : max_size;

    memcpy(buf, con->data + file->f_pos, read_size);
    file->f_pos += read_size;

    return read_size;
}

// vnode operations
int cpiofs_lookup(struct vnode *dir_node, struct vnode **target,
                 const char *component_name)
{
    struct cpio_content* con = dir_node->internal;
    if (con->type != CPIO_DIR) {
        printf("[*] cpiofs_lookup: The component \"%s\" is not a directory.\r\n", component_name);
        *target = NULL;
        return 0;
    }
    for (int i = 0; i < con->size; i++) {
        struct cpio_content* child_con = con->child_list[i]->internal;
        if (strcmp(child_con->name, component_name) == 0) {
            *target = con->child_list[i];
            return 0;
        }
    }
    *target = NULL;
    return -1;
}

int cpiofs_create(struct vnode *dir_node, struct vnode **target,
                 const char *component_name)
{
    printf("[*] cpiofs: create not allowed.\r\n");
    return -1;
}

int cpiofs_mkdir(struct vnode* dir_node, struct vnode** target,
                 const char* component_name)
{
    printf("[*] cpiofs: mkdir not allowed.\r\n");
    return -1;
}

int cpiofs_isdir(struct vnode* node)
{
    struct cpio_content* con = node->internal;
    if (con->type == CPIO_DIR) {
        return 1;
    }
    return 0;
}

int cpiofs_getsize(struct vnode* node)
{
    struct cpio_content* con = node->internal;
    return con->size;
}