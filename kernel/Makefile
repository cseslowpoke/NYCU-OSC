# toolchain
CC = clang # C compiler
LD = ld.lld # Linker
OBJCOPY = llvm-objcopy # 
AS = clang # Assembler

# Build dir
BUILD_DIR = ../build
KERNEL_BUILD = $(BUILD_DIR)/kernel
SRC_DIR = ./src

KERNEL_ELF = $(BUILD_DIR)/kernel.elf
KERNEL_IMG = $(BUILD_DIR)/kernel.img
LIBCOMMON = $(BUILD_DIR)/libcommon.a

SRC = $(wildcard $(SRC_DIR)/*.c) $(wildcard $(SRC_DIR)/*.s)
OBJ = $(patsubst $(SRC_DIR)/%.c, $(KERNEL_BUILD)/%.o, $(wildcard $(SRC_DIR)/*.c))
OBJ += $(patsubst $(SRC_DIR)/%.s, $(KERNEL_BUILD)/%.o, $(wildcard $(SRC_DIR)/*.s))

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
CFLAGS = -mcpu=cortex-a53 --target=aarch64-rpi3-elf --sysroot=$(AARCH64) -Wall -nostdlib -ffreestanding -mgeneral-regs-only -g \
				 -I include -I ../common/include
LDFLAGS = -m aarch64elf -T linker.ld --sysroot=$(AARCH64) 
OBJFLAGS = --output-target=aarch64-rpi3-elf -O binary

COUNT = 0

all: $(KERNEL_IMG)

# compile C files
$(KERNEL_BUILD)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(KERNEL_BUILD)
	$(eval COUNT := $(shell expr $(COUNT) + 1))
	@echo "[$(COUNT)/$(TOTAL)] Compiling $< ... "
	@$(CC) $(CFLAGS) -c $< -o $@

$(KERNEL_BUILD)/%.o: $(SRC_DIR)/%.s
	@mkdir -p $(KERNEL_BUILD)
	@$(eval COUNT := $(shell expr $(COUNT) + 1))
	@echo "[$(COUNT)/$(TOTAL)] Compiling $< ... "
	@$(AS) $(CFLAGS) -c $< -o $@

$(LIBCOMMON):
	@$(MAKE) -C ../common/

$(KERNEL_ELF): $(OBJ) $(LIBCOMMON) linker.ld
	@echo "Linking kernel..."
	@$(LD) $(LDFLAGS) -o $@ $(OBJ) $(LIBCOMMON)

$(KERNEL_IMG): $(KERNEL_ELF)
	@echo "Build kernel Image..."
	@$(OBJCOPY) $(OBJFLAGS) $< $@

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean
