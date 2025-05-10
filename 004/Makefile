AS=nasm
CC=gcc
LD=ld

ASFLAGS=-f elf
CFLAGS=-ffreestanding -m32 -nostdlib
LDSCRIPT=link.ld
LDFLAGS=-m elf_i386 -T $(LDSCRIPT)


SRC=src
BIN=bin
ISO=iso

all: $(BIN)/kernel.bin iso_image

$(BIN)/boot.o: $(SRC)/boot.asm
	$(AS) $(ASFLAGS) $< -o $@

$(BIN)/kernel.o: $(SRC)/kernel.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BIN)/kernel.bin: $(BIN)/boot.o $(BIN)/kernel.o
	$(LD) $(LDFLAGS) $^ -o $@

iso_image: $(BIN)/kernel.bin
	mkdir -p $(ISO)/boot/grub
	cp $(BIN)/kernel.bin $(ISO)/boot/kernel.bin
	echo 'set timeout=0' > $(ISO)/boot/grub/grub.cfg
	echo 'set default=0' >> $(ISO)/boot/grub/grub.cfg
	echo 'menuentry "MyOS" { multiboot /boot/kernel.bin }' >> $(ISO)/boot/grub/grub.cfg
	grub-mkrescue -o $(BIN)/myos.iso $(ISO)

run: all
	qemu-system-i386 -cdrom $(BIN)/myos.iso

clean:
	rm -rf $(BIN)/*.o $(BIN)/*.bin $(BIN)/*.iso $(ISO)
