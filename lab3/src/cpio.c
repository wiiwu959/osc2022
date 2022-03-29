#include <cpio.h>
#include <mini_uart.h>
#include <string.h>
#include <stdint.h>
#include <allocator.h>

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
            char* load_addr = 0x1000000;

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