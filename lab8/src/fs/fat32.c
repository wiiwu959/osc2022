#include <fs/fat32.h>
#include <printf.h>
#include <string.h>
#include <allocator.h>
#include <mm.h>
#include <fs/sdhost.h>

struct file_operations fat32_fops = {
    .write = fat32_write,
    .sync = fat32_sync,
    .read = fat32_read
};

struct vnode_operations fat32_vops = {
    .lookup = fat32_lookup,
    .create = fat32_create,
    .mkdir = fat32_mkdir,
    .isdir = fat32_isdir,
    .getsize = fat32_getsize
};

struct filesystem fat32 = {
    .name = "fat32",
    .setup_mount = &fat32_setup_mount,
    .list = NULL
};

static struct boot_mbr mbr;
static struct fat32_boot_sector bootsector;
static uint32_t *fat_table;

struct vnode* new_fat32_vnode(char* name, int type)
{
    struct vnode* node = kmalloc(sizeof(struct vnode));

    node->mount = NULL;
    node->f_ops = &fat32_fops;
    node->v_ops = &fat32_vops;
    node->parent = NULL;

    struct fat32_content* con = kmalloc(sizeof(struct fat32_content));
    con->name = kmalloc(sizeof(char) * (strlen(name) + 1));
    memcpy(con->name, name, strlen(name));
    con->name[strlen(name)] = '\0';
    con->type = type;

    con->size = 0;
    con->capacity = 0;

    con->data = NULL;
    con->child_num = 0;
    if (type == FAT32_DIR) {
        con->child_list = kmalloc(sizeof(struct vnode*) * FAT32_MAX_FILES);
    } else if (type == FAT32_FILE) {
        con->child_list = NULL;
    }

    node->internal = con;
    return node;
}

static void build_file_internal(struct vnode *node)
{
    struct fat32_content *con = node->internal;
    unsigned aligned = (con->size + SECTOR_SIZE - 1) & ~SECTOR_SIZE;
    con->capacity = aligned;
    con->data = kmalloc(aligned);

    void *buf = kmalloc(SECTOR_SIZE);
    int index = con->data_cluster_index;
    unsigned remain_size = con->size;
    char *ptr = con->data;

    while (index < END_OF_CHAIN_THRESHOLD) {
        readblock(con->root_idx + index - 2, buf);
        unsigned int copy_size = remain_size > SECTOR_SIZE ? SECTOR_SIZE : remain_size;
        memcpy(ptr, buf, copy_size);

        index = fat_table[index];
        remain_size -= SECTOR_SIZE;
        ptr += SECTOR_SIZE;
    }

    con->cached = 1;
    kfree(buf);
}

static void build_dir_internal(struct vnode *dir_node)
{
    struct fat32_content *con = dir_node->internal;
    struct short_entry *entry = con->data;

    for (unsigned i = 0; entry[i].name[0]; i++) {
        // deleted entry
        if (entry[i].name[0] == 0xE5) {
            continue;
        }

        // LFN
        if (entry[i].attribute & ATTR_LONG_NAME == ATTR_LONG_NAME) {
            continue;
        }

        // make name        
        char *name = kmalloc(0x10);
        int tail = 0;
        for (int j = 0; j < 8; j++) {
            if (entry[i].name[j] == ' ') {
                break;
            }
            name[tail++] = entry[i].name[j];
        }
        if (entry[i].extension[0] != ' ') {
            name[tail++] = '.';
            for (int j = 0; j < 3; j++) {
                if (entry[i].extension[j] == ' ') {
                    break;
                }
                name[tail++] = entry[i].extension[j];
            }
        }
        name[tail] = '\0';

        struct vnode *child_node;
        struct fat32_content *child_con;
        if (entry[i].attribute & ATTR_DIRECTORY) {
            child_node = new_fat32_vnode(name, FAT32_DIR);
            child_con = child_node->internal;child_con->data = kmalloc(SECTOR_SIZE);
            
            readblock(child_con->root_idx + child_con->data_cluster_index - 2, child_con->data);
            child_con->child_list = kmalloc(sizeof(struct vnode*) * FAT32_MAX_FILES);
        } else {
            child_node = new_fat32_vnode(name, FAT32_FILE);
            child_con = child_node->internal;
            
            child_con->size = entry[i].size;
        }
        child_con->data_cluster_index = entry[i].cluster;
        child_con->root_idx = con->root_idx;
        child_con->cached = 0;
        child_con->entry_idx = i;
        con->child_list[con->child_num++] = child_node;

        child_node->internal = child_con;
        child_node->parent = dir_node;
    }
    con->cached = 1;
}

int fat32_setup_mount(struct filesystem *fs, struct mount *mount)
{
    printf("[*] FAT32: fat32_setup_mount\r\n");
    struct vnode* root_node = new_fat32_vnode("/", FAT32_DIR);
    struct fat32_content* con = root_node->internal;
    root_node->parent = mount->root;

    // Parse the metadata on the SD card
    readblock(0, &mbr);

    if (mbr.boot_record_signature != 0xaa55) {
        printf("[*] FAT32: wrong mbr end_of_sector\r\n");
    }

    uint32_t bootsector_idx = mbr.partitons[0].relative_sector;
    readblock(bootsector_idx, &bootsector);

    // 32 bytes each entry, num_entries is number of entries in whole FAT
    uint32_t num_entries = bootsector.sectors_per_fat * SECTOR_SIZE;
    uint16_t fat_idx = bootsector_idx + bootsector.reserved_sectors;
    fat_table = kmalloc(num_entries);
    for (int i = 0; i < bootsector.sectors_per_fat; i++) {
        readblock(fat_idx + i, fat_table + i * 512);
    }

    uint16_t rootdir_idx = fat_idx + bootsector.num_of_fat_tables * bootsector.sectors_per_fat;
    con->data = kmalloc(SECTOR_SIZE);
    con->entry_idx = 0;
    con->capacity = SECTOR_SIZE;
    con->root_idx = rootdir_idx;
    con->data_cluster_index = 2; // first cluster is Cluster 2
    con->cached = 0;
    con->child_num = 0;
    readblock(rootdir_idx, con->data);

    mount->fs = fs;
    mount->root = root_node;
    
    return 0;
}

// file operations
int fat32_write(struct file* file, const void* buf, size_t len)
{
    struct fat32_content* con = file->vnode->internal;
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

    struct fat32_content* dir_con = file->vnode->parent->internal;
    struct short_entry* entrys = dir_con->data;
    
    struct short_entry* ent = &entrys[con->entry_idx];
    ent->size = con->size;

    // update entry
    writeblock(dir_con->root_idx + dir_con->data_cluster_index - 2, entrys);

    // update data part
    int idx = con->data_cluster_index;
    int remained_size = ent->size;
    int free_block;

    for (int i = 0; remained_size > 0; i++) {
        writeblock(con->root_idx + idx - 2, (char *)con->data + i * SECTOR_SIZE);
        remained_size -= SECTOR_SIZE;
        if (fat_table[idx] == END_OF_CHAIN && remained_size > 0) {

            uint32_t num_entries = bootsector.sectors_per_fat * SECTOR_SIZE * sizeof(uint32_t);
            free_block = -1;
            for (int i = 0; i < num_entries; i++) {
                if (fat_table[i] == 0) {
                    free_block = i;
                    break;
                }
            }
            if (free_block == -1) { 
                return -1; 
            }

            fat_table[idx] = free_block;
            fat_table[free_block] = END_OF_CHAIN;
        }
        idx = fat_table[idx];
    }
    
    // update fat_table
    uint16_t fat_idx = mbr.partitons[0].relative_sector + bootsector.reserved_sectors;
    int offset = free_block / SECTOR_SIZE;
    for (int i = 0; i < offset; i++) {
        writeblock(fat_idx + i, fat_table + i * SECTOR_SIZE);
    }

    return len;
}

int fat32_read(struct file* file, void* buf, size_t len)
{
    struct fat32_content* con = file->vnode->internal;
    int max_size = con->size - file->f_pos;

    if (max_size < 0 || con->type != FAT32_FILE) {
        buf = NULL;
        return 0;
    }
    
    int read_size = (max_size > len) ? len : max_size;

    memcpy(buf, con->data + file->f_pos, read_size);
    file->f_pos += read_size;

    return read_size;
}

void fat32_sync()
{
    
}

// vnode operations
int fat32_lookup(struct vnode *dir_node, struct vnode **target,
                 const char *component_name)
{
    struct fat32_content* con = dir_node->internal;
    if (con->type != FAT32_DIR) { 
        printf("[*] fat32_lookup vnode not a dir.\r\n");
        return -1;
    }

    if (con->cached == 0) {
        build_dir_internal(dir_node);
    }

    for (int i = 0; i < con->child_num; i++) {
        struct fat32_content* child_con = con->child_list[i]->internal;
        if (child_con->cached == 0) {
            if (child_con->type == FAT32_DIR) {
                child_con->data = con->data;
                build_dir_internal(con->child_list[i]);
            } else if (child_con->type == FAT32_FILE) {
                build_file_internal(con->child_list[i]);
            }
        }

        if (!strcmp(child_con->name, component_name)) {
            *target = con->child_list[i];
            return 0;
        }
    }
    *target = NULL;
    return -1;
}

int fat32_create(struct vnode *dir_node, struct vnode **target,
                 const char *component_name)
{
    // check if existed
    fat32_lookup(dir_node, target, component_name);
    if (*target != NULL) {
        printf("The component \"%s\" is already existed.]\r\n", component_name);
        return -1;
    }
    struct vnode* new_node = new_fat32_vnode(component_name, FAT32_FILE);
    new_node->parent = dir_node;

    struct fat32_content* dir_con = dir_node->internal;
    dir_con->child_list[dir_con->child_num] = new_node;

    int free_block, entry_idx;
    struct short_entry* entrys = dir_con->data;
    int entry_num = SECTOR_SIZE / sizeof(struct short_entry);
    for (entry_idx = 0; entry_idx < entry_num; entry_idx++) {
        // deleted entry or unallocated
        if (entrys[entry_idx].name[0] == 0xE5 || entrys[entry_idx].name[0] == 0x00) {
            struct short_entry* ent = &entrys[entry_idx];
            
            // find free block amd point to END_OF_CHAIN
            uint32_t num_entries = bootsector.sectors_per_fat * SECTOR_SIZE * sizeof(uint32_t);
            free_block = -1;
            for (int i = 0; i < num_entries; i++) {
                if (fat_table[i] == 0) {
                    free_block = i;
                    ent->cluster = free_block;
                    fat_table[free_block] = END_OF_CHAIN;
                    break;
                }
            }
            if (free_block == -1) { 
                return -1; 
            }
           
            ent->size = 0;
            ent->attribute = ATTR_ARCHIVE;

            // set name and extension
            memset(ent->name, ' ', 8);
            memset(ent->extension, ' ', 8);

            int name_pos = 0;
            for (int j = 0; j < 8; j++) {
                if (component_name[name_pos] == '.' || component_name[name_pos] == '\0') {
                    break;
                }
                ent->name[j] = component_name[name_pos];
                name_pos++;
            }

            if (component_name[name_pos] == '.') {
                name_pos++;
                for (int j = 0; j < 3; j++) {
                    if (component_name[name_pos] == '\0') {
                        break;
                    }
                    ent->extension[j] = component_name[name_pos];
                    name_pos++;
                }
            }

            
            // update fat_table
            uint16_t fat_idx = mbr.partitons[0].relative_sector + bootsector.reserved_sectors;
            int offset = free_block / SECTOR_SIZE;
            writeblock(fat_idx + offset, fat_table + offset * SECTOR_SIZE);

            // update entry
            writeblock(dir_con->root_idx + dir_con->data_cluster_index - 2, entrys);
            break;
        }
    }

    struct fat32_content* con = new_node->internal;
    con->root_idx = dir_con->root_idx;
    con->data_cluster_index = free_block;
    con->cached = 1;
    con->entry_idx = entry_idx;

    *target = new_node;
    return 0;
}

int fat32_mkdir(struct vnode* dir_node, struct vnode** target,
                 const char* component_name)
{
    // TODO
    return -1;
}

int fat32_isdir(struct vnode* node)
{
    struct fat32_content* con = node->internal;
    if (con->type == FAT32_DIR) {
        return 1;
    }
    return 0;
}

int fat32_getsize(struct vnode* node)
{
    struct fat32_content* con = node->internal;
    return con->size;
}