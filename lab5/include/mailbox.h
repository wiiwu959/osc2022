#ifndef _MAILBOX_H
#define _MAILBOX_H

#define MMIO_BASE       0x3f000000
#define MAILBOX_BASE    MMIO_BASE + 0xb880

#define MAILBOX_READ    MAILBOX_BASE
#define MAILBOX_STATUS  MAILBOX_BASE + 0x18
#define MAILBOX_WRITE   MAILBOX_BASE + 0x20

#define MAILBOX_EMPTY   0x40000000
#define MAILBOX_FULL    0x80000000

#define MAILBOX_CHANNEL 8

struct arm_memory
{
    unsigned int base_addr;
    unsigned int size;
};

int mailbox_call(unsigned char ch, unsigned int *mbox);
unsigned int get_board_revision(void);
struct arm_memory get_arm_memory(void);

#endif  /*_MAILBOX_H */