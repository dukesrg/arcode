CC=arm-none-eabi-gcc
CFLAGS=-fPIE -fno-zero-initialized-in-bss -std=c99 -mcpu=mpcore -fshort-wchar -O3
ASFLAGS=-nostartfiles -nostdlib
LD=arm-none-eabi-gcc
LDFLAGS=-T linker.x -nodefaultlibs -nostdlib -pie
OBJCOPY=arm-none-eabi-objcopy
OBJCOPYFLAGS=

all: code.bin

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

%.ro: %.S
	$(CC) -c -o $@ $< $(ASFLAGS)

%.elf: %.o
	$(LD) -o $@ $^ $(LDFLAGS)

%.bin: %.elf
	$(OBJCOPY) -O binary $^ $@

%.dat: %.ro
	$(OBJCOPY) -O binary $^ $@

.PHONY: clean

clean:
	rm -rf *~ *.o *.elf *.bin *.s *.dat
