CROSS_COMPILE ?= aarch64-linux-gnu-

CFLAGS   = -Wall -nostdlib -nostartfiles -ffreestanding -Iinclude -mcpu=cortex-a53+nofp+nosimd -mgeneral-regs-only -O0 -g
ASMOPS = -Iinclude 

BUILD_DIR = build
SRC_DIR = src

BOOTLOADER_DIR = bootloader
KERNEL_DIR     = kernel

all : kernel8.img bootloader.img

clean :
	rm -rf $(BUILD_DIR) *.img 

$(BUILD_DIR)/%_c.o: $(SRC_DIR)/%.c
	mkdir -p $(@D)
	$(CROSS_COMPILE)gcc $(CFLAGS) -MMD -c $< -o $@

$(BUILD_DIR)/%_s.o: $(SRC_DIR)/%.S
	$(CROSS_COMPILE)gcc $(ASMOPS) -MMD -c $< -o $@

C_FILES = $(wildcard $(SRC_DIR)/*.c)
ASM_FILES = $(wildcard $(SRC_DIR)/*.S)
OBJ_FILES = $(C_FILES:$(SRC_DIR)/%.c=$(BUILD_DIR)/%_c.o)
OBJ_FILES += $(ASM_FILES:$(SRC_DIR)/%.S=$(BUILD_DIR)/%_s.o)

DEP_FILES = $(OBJ_FILES:%.o=%.d)
-include $(DEP_FILES)

# for building kernel
$(BUILD_DIR)/$(KERNEL_DIR)/%_c.o: $(KERNEL_DIR)/%.c
	mkdir -p $(@D)
	$(CROSS_COMPILE)gcc $(CFLAGS) -MMD -c $< -o $@

$(BUILD_DIR)/$(KERNEL_DIR)/%_s.o: $(KERNEL_DIR)/%.S
	$(CROSS_COMPILE)gcc $(ASMOPS) -MMD -c $< -o $@

KERNEL_C_FILES = $(wildcard $(KERNEL_DIR)/*.c)
KERNEL_ASM_FILES = $(wildcard $(KERNEL_DIR)/*.S)
KERNEL_OBJ_FILES = $(KERNEL_C_FILES:$(KERNEL_DIR)/%.c=$(BUILD_DIR)/$(KERNEL_DIR)/%_c.o)
KERNEL_OBJ_FILES += $(KERNEL_ASM_FILES:$(KERNEL_DIR)/%.S=$(BUILD_DIR)/$(KERNEL_DIR)/%_s.o)

kernel8.img: $(KERNEL_DIR)/linker.ld $(KERNEL_OBJ_FILES) $(OBJ_FILES)
	$(CROSS_COMPILE)ld -T $(KERNEL_DIR)/linker.ld -o $(BUILD_DIR)/kernel8.elf $(OBJ_FILES) $(KERNEL_OBJ_FILES)
	$(CROSS_COMPILE)objcopy $(BUILD_DIR)/kernel8.elf -O binary kernel8.img

run: kernel8.img
	qemu-system-aarch64 -M raspi3 -kernel kernel8.img -display none -serial null -serial stdio -initrd ./config/initramfs.cpio -dtb ./config/bcm2710-rpi-3-b-plus.dtb

debug: kernel8.img
	qemu-system-aarch64 -M raspi3 -kernel kernel8.img -display none -serial null -serial stdio -initrd ./config/initramfs.cpio -dtb ./config/bcm2710-rpi-3-b-plus.dtb  -s -S

# for building bootloader
$(BUILD_DIR)/$(BOOTLOADER_DIR)/%_c.o: $(BOOTLOADER_DIR)/%.c
	mkdir -p $(@D)
	$(CROSS_COMPILE)gcc $(CFLAGS) -MMD -c $< -o $@

$(BUILD_DIR)/$(BOOTLOADER_DIR)/%_s.o: $(BOOTLOADER_DIR)/%.S
	$(CROSS_COMPILE)gcc $(ASMOPS) -MMD -c $< -o $@

BOOTLOADER_C_FILES = $(wildcard $(BOOTLOADER_DIR)/*.c)
BOOTLOADER_ASM_FILES = $(wildcard $(BOOTLOADER_DIR)/*.S)
BOOTLOADER_OBJ_FILES = $(BOOTLOADER_C_FILES:$(BOOTLOADER_DIR)/%.c=$(BUILD_DIR)/$(BOOTLOADER_DIR)/%_c.o)
BOOTLOADER_OBJ_FILES += $(BOOTLOADER_ASM_FILES:$(BOOTLOADER_DIR)/%.S=$(BUILD_DIR)/$(BOOTLOADER_DIR)/%_s.o)

bootloader.img: $(BOOTLOADER_DIR)/linker.ld $(BOOTLOADER_OBJ_FILES) $(OBJ_FILES)
	$(CROSS_COMPILE)ld -T $(BOOTLOADER_DIR)/linker.ld -o $(BUILD_DIR)/bootloader.elf $(OBJ_FILES) $(BOOTLOADER_OBJ_FILES)
	$(CROSS_COMPILE)objcopy $(BUILD_DIR)/bootloader.elf -O binary bootloader.img

load_d: bootloader.img
	qemu-system-aarch64 -M raspi3 -kernel bootloader.img -display none -serial null -serial stdio -s -S

load: bootloader.img
	qemu-system-aarch64 -M raspi3 -kernel bootloader.img -display none -serial null -serial stdio