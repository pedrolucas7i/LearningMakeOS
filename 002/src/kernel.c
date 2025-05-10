#define VIDEO_MEMORY (char *)0xB8000
#define PORT_KEYBOARD 0x60

static int cursor = 0;

void print_char(char c) {
    volatile char *video = VIDEO_MEMORY;
    video[cursor * 2] = c;
    video[cursor * 2 + 1] = 0x0F; // branco sobre preto
    cursor++;
}

unsigned char read_scancode() {
    unsigned char scancode;
    asm volatile ("inb %1, %0" : "=a"(scancode) : "Nd"(PORT_KEYBOARD));
    return scancode;
}

char scancode_to_ascii(unsigned char sc) {
    const char *layout = "\0\0331234567890-=\b\tqwertyuiop[]\n\0asdfghjkl;'`\0\\zxcvbnm,./\0*";
    if (sc > 0 && sc < 58) {
        return layout[sc];
    }
    return 0;
}

void kernel_main() {
    const char *msg = "Pressione teclas:\n";
    while (*msg) print_char(*msg++);

    unsigned char last_scancode = 0;

    while (1) {
        unsigned char scancode = read_scancode();

        // Ignora se for igual ao último (evita repetição rápida)
        if (scancode == last_scancode)
            continue;

        last_scancode = scancode;

        if (scancode & 0x80) {
            // tecla solta — zera last_scancode pra permitir nova leitura
            last_scancode = 0;
        } else {
            char c = scancode_to_ascii(scancode);
            if (c) {
                print_char(c);
            }
        }
    }
}

