start.o: start.S
	riscv64-unknown-elf-gcc -I$(AM_HOME)/am/arch/riscv64-rocket/include -fno-pic -ffunction-sections -MMD -c start.S -o start.o

-include start.d
