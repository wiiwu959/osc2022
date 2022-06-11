#include <fs/tmpfs.h>
#include <fs/vfs.h>
#include <allocator.h>
#include <mm.h>
#include <printf.h>

struct file_operations tmpfs_fops = {
    .write = tmpfs_write,
    .read = tmpfs_read
};

struct vnode_operations tmpfs_vops = {
    .lookup = tmpfs_lookup,
    .create = tmpfs_create,
    .mkdir = tmpfs_mkdir,
    .isdir = tmpfs_isdir,
    .getsize = tmpfs_getsize
};

struct filesystem tmpfs = {
    .name = "tmpfs",
    .setup_mount = &tmpfs_setup_mount,
    .list = NULL
};

int tmpfs_setup_mount(struct filesystem *fs, struct mount *mount)
{
    struct vnode* root_node;
    if (mount->root == NULL) {
        root_node = new_vnode("/", TMP_DIR);
        root_node->parent = NULL;
    } else {
        // root_node = mount->root;
        root_node = new_vnode("/", TMP_DIR);
        root_node->parent = mount->root;
    }

    root_node->mount = mount;

    mount->fs = fs;
    mount->root = root_node;

    return 0;
}

struct vnode* new_vnode(char* name, int type)
{
    struct vnode* node = kmalloc(sizeof(struct vnode));

    node->mount = NULL;
    node->f_ops = &tmpfs_fops;
    node->v_ops = &tmpfs_vops;
    node->parent = NULL;

    struct content* con = kmalloc(sizeof(struct content));
    con->name = kmalloc(sizeof(char) * (strlen(name) + 1));
    memcpy(con->name, name, strlen(name));
    con->name[strlen(name)] = '\0';
    con->type = type;
    con->size = 0;
    con->capacity = 0;
    con->data = NULL;
    if (type == TMP_DIR) {
        con->child_list = kmalloc(sizeof(struct vnode*) * TMP_MAX_FILES);
    } else if (type == TMP_FILE) {
        con->child_list = NULL;
    }

    node->internal = con;
    return node;
}

int tmpfs_write(struct file* file, const void* buf, size_t len)
{
    struct content* con = file->vnode->internal;
    int total_size = file->f_pos + len;

    // allocate a larger size
    if (total_size > con->capacity) {
        char* new_data = kmalloc(total_size);
        if (con->data != NULL) {
            memcpy(new_data, con->data, con->size);
            kfree(con->data);
        }
        con->data = new_data;
        con->capacity = total_size;
    }

    con->size = total_size;
    memcpy(con->data + file->f_pos, buf, len);
    file->f_pos += len;

    return len;
}

int tmpfs_read(struct file* file, void* buf, size_t len)
{
    struct content* con = file->vnode->internal;
    int max_size = con->size - file->f_pos;

    if (max_size < 0 || con->type != TMP_FILE) {
        buf = NULL;
        return 0;
    }
    
    int read_size = (max_size > len) ? len : max_size;

    memcpy(buf, con->data + file->f_pos, read_size);
    file->f_pos += read_size;

    return read_size;
}

// find the component under the specific dir
int tmpfs_lookup(struct vnode *dir_node, struct vnode **target,
                 const char *component_name)
{
    struct content* con = dir_node->internal;
    if (con->type != TMP_DIR) {
        printf("[*] tmpfs_lookup: The component \"%s\" is not a directory.\r\n", component_name);
        *target = NULL;
        return 0;
    }
    for (int i = 0; i < con->size; i++) {
        struct content* child_con = con->child_list[i]->internal;
        if (!strcmp(child_con->name, component_name)) {
            *target = con->child_list[i];
            return 0;
        }
    }
    *target = NULL;
    return -1;
}

int tmpfs_create(struct vnode *dir_node, struct vnode **target,
                 const char *component_name)
{
    // check if existed
    tmpfs_lookup(dir_node, target, component_name);
    if (*target != NULL) {
        printf("The component \"%s\" is already existed.]\r\n", component_name);
        return -1;
    }

    struct vnode* new_node = new_vnode(component_name, TMP_FILE);
    new_node->parent = dir_node;

    struct content* dir_con = dir_node->internal;
    dir_con->child_list[dir_con->size++] = new_node;

    *target = new_node;
    return 0;
}

int tmpfs_mkdir(struct vnode* dir_node, struct vnode** target,
                 const char* component_name)
{
    // check if existed
    tmpfs_lookup(dir_node, target, component_name);
    if (*target != NULL) {
        printf("The component \"%s\" is already existed.\r\n", component_name);
        return -1;
    }

    struct vnode* new_node = new_vnode(component_name, TMP_DIR);
    new_node->parent = dir_node;
    struct content* dir_con = dir_node->internal;
    dir_con->child_list[dir_con->size++] = new_node;
    
    *target = new_node;
    return 0;
}

int tmpfs_isdir(struct vnode* node)
{
    struct content* con = node->internal;
    if (con->type == TMP_DIR) {
        return 1;
    }
    return 0;
}

int tmpfs_getsize(struct vnode* node)
{
    struct content* con = node->internal;
    return con->size;
}