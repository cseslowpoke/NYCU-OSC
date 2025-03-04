all:
	@$(MAKE) -C common
	@$(MAKE) -C kernel

clean:
	@$(MAKE) -C common clean
	@$(MAKE) -C kernel clean

run: all
	qemu-system-aarch64 -M raspi3b -kernel build/kernel.img -display none -serial null -serial stdio 

run_gdb: all
	 qemu-system-aarch64 -M raspi3b -kernel build/kernel.img -display none -serial null -serial stdio -S -s
	
.PHONY: all clean
