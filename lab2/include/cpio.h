#ifndef _CPIO_H
#define _CPIO_H

#include <stdint.h>

// https://www.freebsd.org/cgi/man.cgi?query=cpio&sektion=5
struct cpio_newc_header {
    char c_magic[6];
    char c_ino[8];
    char c_mode[8];
    char c_uid[8];
    char c_gid[8];
    char c_nlink[8];
    char c_mtime[8];
    char c_filesize[8];
    char c_devmajor[8];
    char c_devminor[8];
    char c_rdevmajor[8];
    char c_rdevminor[8];
    char c_namesize[8];
    char c_check[8];
};

unsigned int cpio_read_8hex(char* num);
int cpio_align(int num, int base);
void cpio_list(uint64_t initramfs_loc);
void cpio_cat(char* catfile, uint64_t initramfs_loc);

#endif  /*_CPIO_H */