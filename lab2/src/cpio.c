#include <cpio.h>
#include <mini_uart.h>
#include <string.h>

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

void cpio_list()
{
    struct cpio_newc_header *ramfs = (struct cpio_newc_header *)CPIO_LOC;
    char* ptr = (char*)ramfs;
    while (1) {
        ramfs = (struct cpio_newc_header *)ptr;
        if (strncmp("070701", ramfs->c_magic, 6)) {
            uart_sendline("[*] Not new ASCII format cpio archive.");
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

        uart_sendline(filename);
        uart_sendline("\r\n");
    }
}

void cpio_cat(char* catfile)
{
    struct cpio_newc_header *ramfs = (struct cpio_newc_header *)CPIO_LOC;
    char* ptr = (char*)ramfs;
    while (1) {
        ramfs = (struct cpio_newc_header *)ptr;
        if (strncmp("070701", ramfs->c_magic, 6)) {
            uart_sendline("[*] Not new ASCII format cpio archive.");
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
        } else if (!strcmp(catfile, filename)) {
            ptr += namesize + name_align;
            uart_sendn(ptr, filesize);
            uart_sendline("\r\n");
            break;
        }
        ptr += namesize + name_align + filesize + file_align;
    }
}