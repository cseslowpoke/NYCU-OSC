# toolchain
CC = clang # C compiler
LD = ld.lld # Linker
OBJCOPY = llvm-objcopy # 
AS = clang # Assembler

# Build dir
BUILD_DIR = build
SRC_DIR = src

# 
KERNEL_ELF = $(BUILD_DIR)/kernel.elf
KERNEL_IMG = $(BUILD_DIR)/kernel.img

SRC = $(wildcard $(SRC_DIR)/*.c) $(wildcard $(SRC_DIR)/*.S)

OBJ = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(wildcard $(SRC_DIR)/*.c))
OBJ += $(patsubst $(SRC_DIR)/%.S, $(BUILD_DIR)/%.o, $(wildcard $(SRC_DIR)/*.S))

# Compiler flags
CFLAGS = -mcpu=cortex-a53 --target=aarch64-rpi3-elf 
LDFLAGS = -m aarch64elf -T linker.ld 


all: $(KERNEL_IMG)

# compile C files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.S
	@mkdir -p $(BUILD_DIR)
	$(AS) $(CFLAGS) -c $< -o $@

$(KERNEL_ELF): $(OBJ)
	$(LD) $(LDFLAGS) -o $@ $(OBJ)

$(KERNEL_IMG): $(KERNEL_ELF)
	$(OBJCOPY) -O binary $< $@

clean:
	rm -rf $(BUILD_DIR)

run:
	qemu-system-aarch64 -M raspi3b -kernel $(KERNEL_IMG) -display none -d in_asm

.PHONY: all clean run
