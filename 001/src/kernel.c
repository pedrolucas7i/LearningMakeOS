void printf(const char *msg) {
    volatile char *video = (volatile char*) 0xB8000;
    int i = 0;
    while (msg[i]) {
        video[i * 2] = msg[i];      // caractere
        video[i * 2 + 1] = 0x0F;    // cor: branco sobre preto
        i++;
    }
}



void outb(unsigned short port, unsigned char val) {
    asm volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

void kernel_main() {
    outb(0x3F8, 'X'); // tenta escrever no COM1 — veja no QEMU
    printf("Welcome to MyOS!");
    while (1);
}
