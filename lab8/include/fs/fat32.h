#ifndef _FAT32_H
#define _FAT32_H

#include <fs/vfs.h>
#include <list.h>

#define ATTR_READ_ONLY  0x1
#define ATTR_HIDDEN     0x2
#define ATTR_SYSTEM     0x4
#define ATTR_VOLUME_ID  0x8
#define ATTR_DIRECTORY  0x10
#define ATTR_ARCHIVE    0x20
#define ATTR_LONG_NAME  0xf

#define END_OF_CHAIN 0x0fffffff
#define END_OF_CHAIN_THRESHOLD 0x0ffffff8

// FAT32 relative structure
struct partition_entry {
    char boot_indicator;
    
    uint8_t starting_head;
    uint16_t starting_sector_cylinder;

    uint8_t type; // partition 0 -> 0xb 32-bit FAT, 0xc uses LBA1 13h Extensions

    uint8_t ending_head;
    uint16_t ending_sector_cylinder;

    uint32_t relative_sector;
    uint32_t total_sectors;
} __attribute__((packed));

struct boot_mbr {
    char boot_code[446];
    struct partition_entry partitons[4];
    uint16_t boot_record_signature; // 0xaa55
} __attribute__((packed));


struct fat32_boot_sector {
    char jmp_asm[3];
    char oem_name[8];
    uint16_t bytes_per_sector;
    uint8_t sectors_per_cluster;
    uint16_t reserved_sectors;
    uint8_t num_of_fat_tables;
    uint16_t max_root_dir_entries;
    uint16_t num_of_sectors16; // Number of Sectors inPartition Smaller than 32MB (N/A for FAT32)
    uint8_t media_descriptor; // 0xf0=removable disk, 0xf8=fixed disk

    uint16_t sectors_per_fat16; // in older fs
    uint16_t sectors_per_track;
    uint16_t num_of_heads;
    uint32_t num_of_hidden_sector;
    uint32_t num_of_sector_partition; // Number of Sectors in Partition
    uint32_t sectors_per_fat; // Number of Sectors Per FAT

    /* Flags
    (Bits 0-4 IndicateActive FAT Copy) 
    (Bit 7 Indicates whether FAT Mirroringis Enabled or Disabled)
    (If FATMirroring is Disabled, the FAT Information is onlywritten to the copy indicated by bits 0-4) */
    uint16_t flags;
    // uint16_t active_fat_idx;
    // uint16_t flags_reserved1;
    // uint16_t transaction_fat;
    // uint16_t flags_reserved2;

    uint16_t version;
    uint32_t root_cluster;
    uint16_t sector_num_fs;
    uint16_t sector_num_backupboot;
    char reserved[12];

    uint8_t drive_num;
    uint8_t not_used;
    uint8_t extended_signature; // 0x29, validate next three fields
    uint32_t vol_serial_num;
    char vol_label[11];
    char fs_type[8];
    char executable_code[420];
    char boot_record_sig[2]; // 0xaa55
} __attribute__((packed));

struct short_entry {
    char name[8];
    char extension[3];

    uint8_t attribute;
    uint8_t reserved;
    uint8_t create_time_10ms;

    uint16_t create_time;
    uint16_t create_date;
    uint16_t access_date;

    uint16_t high_cluster;
    uint16_t update_time;
    uint16_t update_date;

    uint16_t cluster;
    uint32_t size;
} __attribute__((packed));

struct fat32_metadata {

} __attribute__((packed));

#define FAT32_DIR   0040000
#define FAT32_FILE  0000000

#define FAT32_MAX_FILES 16
#define SECTOR_SIZE     512

struct fat32_content {
    char* name;
    int type;

    // size: data size
    size_t size;
    size_t capacity;

    // fat info
    int root_idx;
    int data_cluster_index;
    int cached;
    int entry_idx;

    char* data;
    int child_num;
    struct vnode** child_list;
};

extern struct filesystem fat32;

int fat32_setup_mount(struct filesystem *fs, struct mount *mount);

// file operations
int fat32_write(struct file* file, const void* buf, size_t len);
int fat32_read(struct file* file, void* buf, size_t len);
void fat32_sync(void);

// vnode operations
int fat32_lookup(struct vnode *dir_node, struct vnode **target,
                 const char *component_name);
int fat32_create(struct vnode *dir_node, struct vnode **target,
                 const char *component_name);
int fat32_mkdir(struct vnode* dir_node, struct vnode** target,
                 const char* component_name);
int fat32_isdir(struct vnode* node);
int fat32_getsize(struct vnode* node);
 

#endif  /* _FAT32_H */