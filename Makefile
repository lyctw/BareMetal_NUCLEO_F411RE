CC = arm-none-eabi-gcc
MACH = cortex-m4
CFLAGS = -c -mcpu=cortex-m4 -mthumb -std=gnu11 -O0 -Wall
LDFLAGS = -nostdlib -T stm32_ls.ld


all: stm32_startup.o main.o final.elf

main.o: main.c
	$(CC) $(CFLAGS) $^ -o $@


stm32_startup.o: stm32_startup.c
	$(CC) $(CFLAGS) $^ -o $@


final.elf: *.o
	$(CC) $(LDFLAGS) $^ -o $@

clean: 
	rm -rf *.o *.elf

dump:
	arm-none-eabi-objdump -h final.elf
