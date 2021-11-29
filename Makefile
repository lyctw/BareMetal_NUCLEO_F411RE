CC = arm-none-eabi-gcc
MACH = cortex-m4
CFLAGS = -c -mcpu=$(MACH) -mthumb -mfloat-abi=soft -std=gnu11 -Og -g -Wall
LDFLAGS = -nostdlib -T stm32_ls.ld -Wl,-Map=final.map
# LDFLAGS= -mcpu=$(MACH) -mthumb -mfloat-abi=soft -nostdlib --specs=nano.specs -T stm32_ls.ld -Wl,-Map=final.map

all: main.o led.o stm32_startup.o final.elf

main.o: main.c
	$(CC) $(CFLAGS) $^ -o $@

led.o: led.c
	$(CC) $(CFLAGS) $^ -o $@

stm32_startup.o: stm32_startup.c
	$(CC) $(CFLAGS) $^ -o $@

final.elf: *.o
	$(CC) $(LDFLAGS) $^ -o $@

clean: 
	rm -rf *.o *.elf

dump:
	arm-none-eabi-objdump -h final.elf

print-symbol-table:
	arm-none-eabi-nm final.elf

load:
	openocd -f st_nucleo_f4.cfg
