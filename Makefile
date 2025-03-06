all:
	@echo "===== Compiling common ====="
	@$(MAKE) -C common --no-print-directory -s
	@echo "===== Compiling bootloader ====="
	@$(MAKE) -C bootloader --no-print-directory -s
	@echo "===== Compiling kernel ====="
	@$(MAKE) -C kernel --no-print-directory -s

clean:
	@$(MAKE) -C common clean --no-print-directory
	@$(MAKE) -C bootloader clean --no-print-directory
	@$(MAKE) -C kernel clean --no-print-directory

bootloader: all
	@qemu-system-aarch64 -M raspi3b -kernel build/bootloader.img -display none -serial null -serial stdio

bootloader_gdb: all
	@qemu-system-aarch64 -M raspi3b -kernel build/bootloader.img -display none -serial null -serial stdio -S -s

run: all
	@qemu-system-aarch64 -M raspi3b -kernel build/kernel.img -display none -serial null -serial stdio -initrd initramfs.cpio
run_gdb: all
	@qemu-system-aarch64 -M raspi3b -kernel build/kernel.img -display none -serial null -serial stdio -initrd initramfs.cpio -S -s
	
.PHONY: all clean bootloader bootloader_gdb run run_gdb
