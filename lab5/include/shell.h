#ifndef _SHELL_H
#define _SHELL_H

void cmd_help(void);
void cmd_hello(void);
void cmd_reboot(void);
void cmd_hardware(void);
void cmd_ls(void);
void cmd_cat(void);
void cmd_load(void);
void cmd_malloc(void);
void cmd_async(void);
void cmd_settimeout(char* buffer);
void cmd_hint(void);

void shell(void);

#endif  /*_SHELL_H */