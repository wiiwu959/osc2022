#include <cpio.h>
#include <mini_uart.h>
#include <string.h>
#include <stdint.h>
#include <allocator.h>
#include <fdt.h>

unsigned int cpio_read_8hex(char* num)
{
    unsigned int result = 0;
    for (int i = 0; i < 8; i++) {
        result <<= 4;
        if (num[i] >= '0' && num[i] <= '9') {
            result += num[i] - '0';
        } else if (num[i] >= 'A' && num[i] <= 'F') {
            result += 10 + num[i] - 'A';
        } else if (num[i] >= 'a' && num[i] <= 'f') {
            result += 10 + num[i] - 'a';
        }
    }
    return result;
}

int cpio_align(int num, int base)
{
    if (num % base != 0 ) {
        return base - num % base;
    } 
    return 0;
}

void cpio_list(uint64_t initramfs_loc)
{
    struct cpio_newc_header *ramfs = (struct cpio_newc_header *)initramfs_loc;
    char* ptr = (char*)initramfs_loc;
    while (1) {
        ramfs = (struct cpio_newc_header *)ptr;
        if (strncmp("070701", ramfs->c_magic, 6)) {
            uart_send_string("[*] Not new ASCII format cpio archive.");
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
        }

        ptr += namesize + name_align + filesize + file_align;

        uart_send_string(filename);
        uart_send_string("\r\n");
    }
}

void cpio_cat(char* catfile, uint64_t initramfs_loc)
{
    struct cpio_newc_header *ramfs = (struct cpio_newc_header *)initramfs_loc;
    char* ptr = (char*)ramfs;
    while (1) {
        ramfs = (struct cpio_newc_header *)ptr;
        if (strncmp("070701", ramfs->c_magic, 6)) {
            uart_send_string("[*] Not new ASCII format cpio archive.\r\n");
            break;
        }
        
        unsigned int namesize = cpio_read_8hex(ramfs->c_namesize);
        unsigned int filesize = cpio_read_8hex(ramfs->c_filesize);
        int name_align = cpio_align(sizeof(struct cpio_newc_header) + namesize, 4);
        int file_align = cpio_align(filesize, 4);
        
        ptr += sizeof(struct cpio_newc_header);
        char* filename = ptr;
        if (!strcmp("TRAILER!!!", filename)) {
            uart_send_string("[*] No such file.\r\n");
            break;
        } else if (!strcmp(catfile, filename)) {
            ptr += namesize + name_align;
            uart_sendn(ptr, filesize);
            uart_send_string("\r\n");
            break;
        }
        ptr += namesize + name_align + filesize + file_align;
    }
}

void cpio_exec(char* getfile, uint64_t initramfs_loc)
{
    struct cpio_newc_header *ramfs = (struct cpio_newc_header *)initramfs_loc;
    char* ptr = (char*)ramfs;
    while (1) {
        ramfs = (struct cpio_newc_header *)ptr;
        if (strncmp("070701", ramfs->c_magic, 6)) {
            uart_send_string("[*] Not new ASCII format cpio archive.");
            break;
        }
        
        unsigned int namesize = cpio_read_8hex(ramfs->c_namesize);
        unsigned int filesize = cpio_read_8hex(ramfs->c_filesize);
        int name_align = cpio_align(sizeof(struct cpio_newc_header) + namesize, 4);
        int file_align = cpio_align(filesize, 4);
        
        ptr += sizeof(struct cpio_newc_header);
        char* filename = ptr;
        if (!strcmp("TRAILER!!!", filename)) {
            uart_send_string("[*] No such file.\r\n");
            break;
        } else if (!strcmp(getfile, filename)) {
            ptr += namesize + name_align;
            char* load_addr = (char*)0x1000000;

            char* content = ptr;
            char* loading = load_addr;

            while (filesize--) {
                *loading = *content;
                content++;
                loading++;
            }

            asm volatile (
                "mov x10, 0 \n\t"
                "msr spsr_el1, x10 \n\t"
                "msr elr_el1, %0 \n\t" 
                "msr sp_el0, %1 \n\t"
                "eret \n\t"
                ::  "r" (load_addr),
                    "r" (load_addr + 0x1000)
            );
            
            break;
        }
        ptr += namesize + name_align + filesize + file_align;
    }
}

// for kernel init
void initramfs_init(char* fdt)
{
    initramfs_loc = 0;
    fdt_end = fdt_traverse(fdt, initramfs_callback);
}

void initramfs_callback(char* fdt, char* node) 
{
    if (strcmp(node, "chosen") && strcmp(node, "memory@0")) {
        return;
    }

    char* cur = node;
    cur += fdt_alignup(strlen(node) + 1, 4);
    char* dt_string = fdt_get_dt_string(fdt);

    while (fdt_get_uint32(cur) == FDT_PROP) {
        cur += 4;
        struct fdt_prop* prop = (struct fdt_prop*)cur;
        uint32_t prop_len = fdt_get_uint32((char*)&prop->len);
        char* prop_name = dt_string + fdt_get_uint32((char*)&prop->nameoff);

        cur += sizeof(struct fdt_prop);
        if (!strcmp(prop_name, "linux,initrd-start")) {
            uart_send_string("[*] cpio archive file loaded at: ");
            initramfs_loc = fdt_get_uint32(cur);
            uart_send_hex(initramfs_loc);
            uart_send_string("\r\n");
        } else if (!strcmp(prop_name, "linux,initrd-end")) {
            initramfs_end = fdt_get_uint32(cur);
        } else if (!strcmp(prop_name, "reg")) {
            memory_end = fdt_get_uint64(cur);
        }
        cur += fdt_alignup(prop_len, 4);
    }
    return;
}