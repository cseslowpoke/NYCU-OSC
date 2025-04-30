BUILD_ROOT := ../build
BUILD_USER := $(BUILD_ROOT)/user

CC := clang
LD := ld.lld
OBJCOPY := llvm-objcopy

CFLAGS := -mcpu=cortex-a53 --target=aarch64-rpi3-elf \
					-Wall -nostdlib -ffreestanding \
					-mgeneral-regs-only -g -Iinclude -fPIC -fomit-frame-pointer
LDFLAGS := -T linker.ld
OBJFLAGS = --output-target=aarch64-rpi3-elf -O binary

TARGETS = $(BUILD_USER)/fork.img

all: $(TARGETS)

$(BUILD_USER)/%.img: $(BUILD_USER)/%.elf
	@mkdir -p $(dir $@)
	$(OBJCOPY) $(OBJFLAGS) $< $@

$(BUILD_USER)/%.elf: $(BUILD_USER)/%.o 
	@mkdir -p $(dir $@)
	$(LD) $(LDFLAGS) -o $@ $<

$(BUILD_USER)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_USER)

.PRECIOUS: $(BUILD_USER)/%.elf
.PHONY: all clean
