# toolchain
CC = clang # C compiler
LD = ld.lld # Linker
OBJCOPY = llvm-objcopy # 
AS = clang # Assembler

# Build dir
BUILD_DIR = ./build
SRC_DIR = ./src
INCLUDE_DIR = ./include
# 
KERNEL_ELF = $(BUILD_DIR)/kernel.elf
KERNEL_IMG = $(BUILD_DIR)/kernel.img

SRC = $(wildcard $(SRC_DIR)/*.c) $(wildcard $(SRC_DIR)/*.s)
OBJ = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(wildcard $(SRC_DIR)/*.c))
OBJ += $(patsubst $(SRC_DIR)/%.s, $(BUILD_DIR)/%.o, $(wildcard $(SRC_DIR)/*.s))

STALE_OBJ := $(shell for obj in $(OBJ); do \
                 src="$$(echo $$obj | sed 's|build/|src/|' | sed 's|.o$$||')"; \
                 if [ ! -f $$obj ] || [ -f $$src.c -a $$src.c -nt $$obj ] || [ -f $$src.s -a $$src.s -nt $$obj ]; then \
                   echo $$obj; \
                 fi; \
               done)

TOTAL := $(words $(STALE_OBJ))

# Host library
AARCH64 = /usr/aarch64-linux-gnu/

# Compiler flags
CFLAGS = -mcpu=cortex-a53 --target=aarch64-rpi3-elf --sysroot=$(AARCH64) -Wall -nostdlib -ffreestanding -mgeneral-regs-only -g -I$(INCLUDE_DIR)
LDFLAGS = -m aarch64elf -T linker.ld --sysroot=$(AARCH64) 
OBJFLAGS = --output-target=aarch64-rpi3-elf -O binary

all: $(KERNEL_IMG)

# compile C files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(BUILD_DIR)
	@$(eval COUNT := $(shell expr $(COUNT) + 1))
	@echo "[$(COUNT)/$(TOTAL)] Compiling $< ... "
	@$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.s
	@mkdir -p $(BUILD_DIR)
	@$(eval COUNT := $(shell expr $(COUNT) + 1))
	@echo "[$(COUNT)/$(TOTAL)] Compiling $< ... "
	@$(AS) $(CFLAGS) -c $< -o $@

$(KERNEL_ELF): $(OBJ)
	@echo "Linking kernel..."
	@$(LD) $(LDFLAGS) -o $@ $(OBJ)
	
$(KERNEL_IMG): $(KERNEL_ELF)
	@echo "Build kernel Image..."
	@$(OBJCOPY) $(OBJFLAGS) $< $@

clean:
	rm -rf $(BUILD_DIR)

run:
	qemu-system-aarch64 -M raspi3b -kernel $(KERNEL_IMG) -display none -d in_asm

.PHONY: all clean run
