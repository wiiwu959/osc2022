#include <util.h>
#include <mailbox.h>

// Tag
#define GET_BOARD_REVISION  0x00010002
#define GET_ARM_MEMORY      0x00010005

#define REQUEST_CODE        0x00000000
#define REQUEST_SUCCEED     0x80000000
#define REQUEST_FAILED      0x80000001
#define TAG_REQUEST_CODE    0x00000000
#define END_TAG             0x00000000

volatile unsigned int __attribute__((aligned(16))) mailbox[36];

int mailbox_call(unsigned char ch, unsigned int *mbox)
{
    // Combine the message address (upper 28 bits) with channel number (lower 4 bits)
    unsigned int r = (((unsigned int)((unsigned long)mbox) & ~0xF) | (ch & 0xF));

    // Check if Mailbox 0 status register’s full flag is set.
    // If not, then you can write to Mailbox 1 Read/Write register.
    while (get(MAILBOX_STATUS) & MAILBOX_FULL) {}
    put(MAILBOX_WRITE, r);
    
    // Check if Mailbox 0 status register’s empty flag is set.
    // If not, then you can read from Mailbox 0 Read/Write register.
    while (get(MAILBOX_STATUS) & MAILBOX_EMPTY) {}
    // Check if the value is the same as you wrote in step 1.
    return get(MAILBOX_READ) == r;
}

// https://github.com/raspberrypi/firmware/wiki/Mailbox-property-interface
// Length: 8, u64: board serial
unsigned int get_board_revision()
{
    // unsigned int mailbox[7];
    mailbox[0] = 7 * 4; // buffer size in bytes
    mailbox[1] = REQUEST_CODE;
    // tags begin
    mailbox[2] = GET_BOARD_REVISION; // tag identifier
    mailbox[3] = 4; // maximum of request and response value buffer's length.
    mailbox[4] = TAG_REQUEST_CODE;
    mailbox[5] = 0; // value buffer
    // tags end
    mailbox[6] = END_TAG;

    mailbox_call(MAILBOX_CHANNEL, mailbox); // message passing procedure call, you should implement it following the 6 steps provided above.

    return mailbox[5]; // it should be 0xa020d3 for rpi3 b+
}

// Length: 8, u32: base address in bytes, u32: size in bytes
// ARM memory base address and size
struct arm_memory get_arm_memory()
{
    // unsigned int mailbox[8];
    mailbox[0] = 8 * 4; // buffer size in bytes
    mailbox[1] = REQUEST_CODE;
    // tags begin
    mailbox[2] = GET_ARM_MEMORY; // tag identifier
    mailbox[3] = 8; // maximum of request and response value buffer's length.
    mailbox[4] = TAG_REQUEST_CODE;
    mailbox[5] = 0; // base address value buffer
    mailbox[6] = 0; // size value buffer
    // tags end
    mailbox[7] = END_TAG;

    mailbox_call(MAILBOX_CHANNEL, mailbox);

    struct arm_memory result = { .base_addr = mailbox[5], .size = mailbox[6]};
    
    return result;
}