PROJECT = shc

CC = arm-none-eabi-gcc
CPU = cortex-m3
FLAGS = -mthumb -mcpu=$(CPU) -c -g3 -O0 -std=gnu11
LD = arm-none-eabi-ld
GDB = arm-none-eabi-gdb

LDFLAGS = -T map.ld
LDFLAGS2 = -nostdlib -T map.ld -Map=out.map

BOARD ?= stm32vldiscovery

.PHONY: all test qemu gdb clean

all: $(PROJECT).elf

o_files/stm32_startup.o: stm32_startup.c
	$(CC) $(FLAGS) -o $@ $^

o_files/main.o: main.c
	$(CC) $(FLAGS) -o $@ $^

o_files/task.o: tasks/tasks.c
	$(CC) $(FLAGS) -o $@ $^

o_files/m_heap.o: dump_heap/my_heap.c
	$(CC) $(FLAGS) -o $@ $^


$(PROJECT).elf: o_files/main.o o_files/stm32_startup.o o_files/task.o o_files/m_heap.o
	$(LD) $(LDFLAGS2) -o $@ $^

test: $(PROJECT).elf

qemu: $(PROJECT).elf
	arm-none-eabi-objdump -D -S $(PROJECT).elf > $(PROJECT).elf.lst
	arm-none-eabi-readelf -a $(PROJECT).elf > $(PROJECT).elf.debug
	qemu-system-arm -S -M $(BOARD) -cpu $(CPU) -nographic -kernel $(PROJECT).elf -gdb tcp::1234

gdb: $(PROJECT).elf
	$(GDB) -q $(PROJECT).elf -ex "target remote localhost:1234"

clean:
	rm -f *.o *.elf *.map *.lst *.debug