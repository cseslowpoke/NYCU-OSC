CC = clang
LD = ld.lld
OBJCOPY = llvm-objcopy
AS = clang


BUILD_DIR = ../build
BOOTLOADER_BUILD = ../build/bootloader

KERNEL_ELF = $(BUILD_DIR)/bootloader.elf
KERNEL_IMG = $(BUILD_DIR)/bootloader.img
LIBCOMMON = $(BUILD_DIR)/libcommon.a

SRC = boot.s main.c
OBJ = $(BOOTLOADER_BUILD)/boot.o $(BOOTLOADER_BUILD)/main.o

STALE_OBJ := $(shell for obj in $(OBJ); do \
                 src="$$(echo $$obj | sed 's|build/|src/|' | sed 's|.o$$||')"; \
                 if [ ! -f $$obj ] || [ -f $$src.c -a $$src.c -nt $$obj ] || [ -f $$src.s -a $$src.s -nt $$obj ]; then \
                   echo $$obj; \
                 fi; \
               done)

TOTAL := $(words $(STALE_OBJ))


AARCH64 = /usr/aarch64-linux-gnu/
CFLAGS = -mcpu=cortex-a53 --target=aarch64-rpi3-elf --sysroot=$(AARCH64) -Wall -nostdlib -ffreestanding -mgeneral-regs-only -g \
				 -I ../common/include
LDFLAGS = -m aarch64elf -T linker.ld --sysroot=$(AARCH64) 
OBJFLAGS = --output-target=aarch64-rpi3-elf -O binary

COUNT = 0

all: $(KERNEL_IMG)

# compile C files
$(BOOTLOADER_BUILD)/%.o: %.c
	@mkdir -p $(BUILD_DIR)
	@mkdir -p $(BOOTLOADER_BUILD)
	@$(eval COUNT := $(shell expr $(COUNT) + 1))
	@echo "[$(COUNT)/$(TOTAL)] Compiling $< ... "
	@$(CC) $(CFLAGS) -c $< -o $@

$(BOOTLOADER_BUILD)/%.o: %.s
	@mkdir -p $(BUILD_DIR)
	@mkdir -p $(BOOTLOADER_BUILD)
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
	rm -rf $(BOOTLOADER_BUILD) $(KERNEL_ELF) $(KERNEL_IMG)

.PHONY: all clean
