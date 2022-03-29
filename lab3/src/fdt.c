#include <fdt.h>
#include <stdint.h>
#include <string.h>
#include <mini_uart.h>

uint32_t fdt_get_uint32(char* ptr)
{
    uint32_t result = 0;
    for (int i = 0; i < 4; i++) {
        result <<= 8;
        result += ptr[i];
    }
    return result;
}

char* fdt_get_dt_string(char* fdt)
{
    struct fdt_header* header = (struct fdt_header*)fdt;
    return fdt + fdt_get_uint32((char*)&header->off_dt_strings);
}

char* fdt_get_dt_struct(char* fdt)
{
    struct fdt_header* header = (struct fdt_header*)fdt;
    return fdt + fdt_get_uint32((char*)&header->off_dt_struct);
}

int fdt_alignup(int num, int base)
{
    if (num % base != 0 ) {
        return base - num % base + num;
    } 
    return num;
}

void fdt_traverse(char* fdt, void (*callback)(char*, char*))
{
    struct fdt_header* header = (struct fdt_header*)fdt;
    if (fdt_get_uint32((char*)&header->magic) != 0xd00dfeed) {
        uart_send_string("\r\n");
        uart_send_hex(fdt_get_uint32((char*)&header->magic));
        uart_send_string("[*] Magic number wrong.\r\n");
    }

    char* cur = fdt_get_dt_struct(fdt);
    int layer = 0;
    while (1) {
        uint32_t fdt_token = fdt_get_uint32(cur);
        cur += 4;
        if (fdt_token == FDT_BEGIN_NODE) {
            
            if (!strcmp(cur, "chosen")) {
                callback(fdt, cur);
                break;
            }

            cur += fdt_alignup(strlen(cur) + 1, 4);
            layer++;
        } else if (fdt_token == FDT_END_NODE) {
            layer--;
        } else if (fdt_token == FDT_PROP) {
            struct fdt_prop* prop = (struct fdt_prop*)cur;
            uint32_t prop_len = fdt_get_uint32((char*)&prop->len);
            cur += sizeof(struct fdt_prop) + fdt_alignup(prop_len, 4);
        } else if (fdt_token == FDT_END) {
            if (layer != 0) {
                uart_send_string("[*] dtb file corrupted.\r\n");
            }
            break;
        } else if (fdt_token == FDT_NOP) {
            continue;
        } 
        else {
            uart_send_string("[*] dtb file corrupted.\r\n");
            break;
        }
    }
}

void fdt_list(char* fdt)
{
    char* cur = fdt_get_dt_struct(fdt);
    int layer = 0;
    char* dt_string = fdt_get_dt_string(fdt);
    while (1) {
        uint32_t fdt_token = fdt_get_uint32(cur);
        cur += 4;
        if (fdt_token == FDT_BEGIN_NODE) {
            for (int i = 0; i < layer; i++) {
                uart_send_string("    ");
            }
            uart_send_string("{\r\n");

            int align = fdt_alignup(strlen(cur) + 1, 4);
            cur += align;
            layer++;
        } else if (fdt_token == FDT_END_NODE) {
            layer--;
            for (int i = 0; i < layer; i++) {
                uart_send_string("    ");
            }
            uart_send_string("}\r\n");
        } else if (fdt_token == FDT_PROP) {
            struct fdt_prop* prop = (struct fdt_prop*)cur;
            
            uint32_t prop_len = fdt_get_uint32((char*)&prop->len);
            char* prop_name = dt_string + fdt_get_uint32((char*)&prop->nameoff);
            for (int i = 0; i < layer; i++) {
                uart_send_string("    ");
            }
            uart_send_string(prop_name);
            uart_send_string("\r\n");

            int prop_align = fdt_alignup(prop_len, 4);
            cur += sizeof(struct fdt_prop) + prop_align;
        } else if (fdt_token == FDT_END) {
            if (layer != 0) {
                uart_send_string("[*] dtb file corrupted.\r\n");
            }
            break;
        } else if (fdt_token == FDT_NOP) {
            continue;
        } 
        else {
            uart_send_string("[*] dtb file corrupted.\r\n");
            break;
        }
    }
}