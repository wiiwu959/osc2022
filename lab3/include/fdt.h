#ifndef _FDT_H
#define _FDT_H

#include <stdint.h>

#define FDT_BEGIN_NODE  0x00000001
#define FDT_END_NODE    0x00000002
#define FDT_PROP        0x00000003
#define FDT_NOP         0x00000004
#define FDT_END         0x00000009

struct fdt_header {
    uint32_t magic;
    uint32_t totalsize;
    uint32_t off_dt_struct;
    uint32_t off_dt_strings;
    uint32_t off_mem_rsvmap;
    uint32_t version;
    uint32_t last_comp_version;
    uint32_t boot_cpuid_phys;
    uint32_t size_dt_strings;
    uint32_t size_dt_struct;
};

struct fdt_prop {
    uint32_t len;
    uint32_t nameoff;
};

uint32_t fdt_get_uint32(char* ptr);
char* fdt_get_dt_string(char* fdt);
char* fdt_get_dt_struct(char* fdt);
int fdt_alignup(int num, int base);
void fdt_traverse(char* fdt, void (*callback)(char*, char*));
void fdt_list(char* fdt);


#endif  /*_FDT_H */