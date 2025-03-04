# toolchain
CC = clang
AR = llvm-ar

BUILD_DIR = ../build
SRC_DIR = ./src
INCLUDE_DIR = ./include/

# Host library
AARCH64 = /usr/aarch64-linux-gnu/

# compiler flag
CFLAGS = -mcpu=cortex-a53 --target=aarch64-rpi3-elf --sysroot=$(AARCH64) -Wall -nostdlib -ffreestanding -mgeneral-regs-only -g -I$(INCLUDE_DIR)

LIBRARY = $(BUILD_DIR)/libcommon.a

SRC = $(wildcard src/*.c)
OBJ = $(patsubst src/%.c, $(BUILD_DIR)/%.o, $(SRC))

STALE_OBJ := $(shell for obj in $(OBJ); do \
                 src="$$(echo $$obj | sed 's|build/|src/|' | sed 's|.o$$||')"; \
                 if [ ! -f $$obj ] || [ -f $$src.c -a $$src.c -nt $$obj ] || [ -f $$src.s -a $$src.s -nt $$obj ]; then \
                   echo $$obj; \
                 fi; \
               done)

TOTAL := $(words $(STALE_OBJ))

all: $(LIBRARY)
	
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(BUILD_DIR)
	@$(eval COUNT := $(shell expr $(COUNT) + 1))
	@echo "[$(COUNT)/$(TOTAL)] Compiling $< ... "
	@$(CC) $(CFLAGS) -c $< -o $@

$(LIBRARY): $(OBJ)
	@$(AR) rcs $(LIBRARY) $(OBJ)

clean:
	rm -rf $(LIBRARY) $(OBJ)

.PHONY: all clean
