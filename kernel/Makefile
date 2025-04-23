BUILD_ROOT := ../build
BUILD_KERNEL := $(BUILD_ROOT)/kernel

CC := clang
LD := ld.lld
OBJCOPY := llvm-objcopy
AS := clang

CFLAGS := -mcpu=cortex-a53 --target=aarch64-rpi3-elf --sysroot=$(AARCH64) -Wall -nostdlib -ffreestanding -mgeneral-regs-only -g -Iinclude
LDFLAGS := -T linker.ld
OBJFLAGS = --output-target=aarch64-rpi3-elf -O binary

include arch/arm64/Makefile
include common/Makefile
include core/Makefile
include drivers/Makefile
include fs/Makefile
include mm/Makefile

SRC := $(SRC_ARCH) $(SRC_COMMON) $(SRC_CORE) $(SRC_DRIVERS) $(SRC_FS) $(SRC_MM) main.c
OBJS := $(patsubst %.c, $(BUILD_KERNEL)/%.o, $(filter %.c, $(SRC))) \
        $(patsubst %.s, $(BUILD_KERNEL)/%.o, $(filter %.s, $(SRC)))

STALE_OBJ := $(shell for obj in $(OBJS); do \
                 src="$$(echo $$obj | sed 's|build/|src/|' | sed 's|.o$$||')"; \
                 if [ ! -f $$obj ] || [ -f $$src.c -a $$src.c -nt $$obj ] || [ -f $$src.s -a $$src.s -nt $$obj ]; then \
                   echo $$obj; \
                 fi; \
               done)

TOTAL := $(words $(STALE_OBJ))
COUNT = 0

all: $(BUILD_ROOT)/kernel.img

$(BUILD_KERNEL)/%.o: %.c
	@mkdir -p $(dir $@)
	@$(eval COUNT := $(shell expr $(COUNT) + 1))
	@echo "[$(COUNT)/$(TOTAL)] Compiling $< ... "
	@$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_KERNEL)/%.o: %.s
	@mkdir -p $(dir $@)
	@$(eval COUNT := $(shell expr $(COUNT) + 1))
	@echo "[$(COUNT)/$(TOTAL)] Compiling $< ... "
	@$(AS) $(CFLAGS) -c $< -o $@

$(BUILD_ROOT)/kernel.elf: $(OBJS) linker.ld
	@echo "Linking kernel..."
	@$(LD) $(LDFLAGS) $(OBJS) -o $@

$(BUILD_ROOT)/kernel.img: $(BUILD_ROOT)/kernel.elf
	@echo "Build kernel Image..."
	@$(OBJCOPY) $(OBJFLAGS) $< $@

clean:
	rm -rf $(BUILD_ROOT)
