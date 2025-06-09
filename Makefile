all:
	@make -C bootloader -s -q || (echo "===== Compiling bootloader =====" && make -C bootloader --no-print-directory -s)
	@make -C kernel -s -q || (echo "===== Compiling kernel =====" && make -C kernel --no-print-directory -s)
	@make -C user -s -q || (echo "===== Compiling user =====" && make -C user --no-print-directory -s)

clean:
	@$(MAKE) -C bootloader clean --no-print-directory
	@$(MAKE) -C kernel clean --no-print-directory
	@$(MAKE) -C user clean --no-print-directory

bootloader: all
	@qemu-system-aarch64 -M raspi3b -kernel build/bootloader.img -display none -serial null -serial stdio -initrd initramfs.cpio -dtb bcm2710-rpi-3-b-plus.dtb

bootloader_gdb: all
	@qemu-system-aarch64 -M raspi3b -kernel build/bootloader.img -display none -serial null -serial stdio -S -s -initrd initramfs.cpio -dtb bcm2710-rpi-3-b-plus.dtb

run: all
	@qemu-system-aarch64 -M raspi3b -kernel build/kernel.img -display none -serial null -serial stdio -initrd initramfs.cpio -dtb bcm2710-rpi-3-b-plus.dtb -d int -D log

run_gdb: all
	@qemu-system-aarch64 -M raspi3b -kernel build/kernel.img -display none -serial null -serial stdio -initrd initramfs.cpio -dtb bcm2710-rpi-3-b-plus.dtb -S -s -d int -D log
	# @qemu-system-aarch64 -M raspi3b -kernel build/kernel.img -serial null -serial stdio -initrd initramfs.cpio -dtb bcm2710-rpi-3-b-plus.dtb -S -s -d int -D log

run_video: all
	@qemu-system-aarch64 -M raspi3b -kernel build/kernel.img -serial null -serial stdio -initrd initramfs.cpio -dtb bcm2710-rpi-3-b-plus.dtb -d int -D log
	
.PHONY: all clean bootloader bootloader_gdb run run_gdb
