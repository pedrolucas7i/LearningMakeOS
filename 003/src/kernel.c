#define VIDEO_MEMORY (char *)0xB8000
#define PORT_KEYBOARD 0x60
#define MAX_INPUT_SIZE 128
#define MAX_HISTORY_SIZE 10

// Definindo size_t manualmente
typedef unsigned int size_t;

// Declaração de clear_screen antes do uso
void clear_screen();

static int cursor = 0;
static char input_buffer[MAX_INPUT_SIZE];
static int input_pos = 0;
static char command_history[MAX_HISTORY_SIZE][MAX_INPUT_SIZE];
static int history_pos = 0;
static int history_index = 0;

void print_char(char c) {
    volatile char *video = VIDEO_MEMORY;
    video[cursor * 2] = c;
    video[cursor * 2 + 1] = 0x0F; // branco sobre preto
    cursor++;
}

void print_string(const char *str) {
    while (*str) {
        print_char(*str++);
    }
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

// Implementações de strcmp, strcpy, e strlen

int strcmp(const char *str1, const char *str2) {
    while (*str1 && (*str1 == *str2)) {
        str1++;
        str2++;
    }
    return (unsigned char)*str1 - (unsigned char)*str2;
}

char *strcpy(char *dest, const char *src) {
    char *start = dest;
    while (*src) {
        *dest = *src;
        dest++;
        src++;
    }
    *dest = '\0'; // Termina a string com o caracter nulo
    return start;
}

size_t strlen(const char *str) {
    size_t len = 0;
    while (*str++) {
        len++;
    }
    return len;
}

void clear_input_buffer() {
    for (int i = 0; i < MAX_INPUT_SIZE; i++) {
        input_buffer[i] = '\0';
    }
    input_pos = 0;
}

void handle_enter() {
    input_buffer[input_pos] = '\0'; // Garantir que a string termine
    print_string("\nComando executado: ");
    print_string(input_buffer);
    
    // Salva o comando no histórico
    if (history_pos < MAX_HISTORY_SIZE) {
        // Evitar duplicar comandos consecutivos no histórico
        if (history_pos == 0 || strcmp(input_buffer, command_history[history_pos - 1]) != 0) {
            strcpy(command_history[history_pos], input_buffer);
            history_pos++;
        }
    }
    
    // Processa o comando
    if (strcmp(input_buffer, "help") == 0) {
        print_string("\nComandos disponíveis:\n");
        print_string("help - Exibe esta lista de comandos\n");
        print_string("clear - Limpa a tela\n");
        print_string("Comandos personalizados podem ser adicionados.\n");
    } else if (strcmp(input_buffer, "clear") == 0) {
        clear_screen();
    } else {
        print_string("\nComando desconhecido: ");
        print_string(input_buffer);
    }
    
    clear_input_buffer(); // Limpa o buffer após executar o comando
    print_char('\n');
}

void clear_screen() {
    // Limpa a tela escrevendo espaços na memória VGA
    for (int i = 0; i < 80 * 25; i++) {
        *((volatile char*)VIDEO_MEMORY + i * 2) = ' ';
    }
    cursor = 0; // Resetar o cursor
}

void handle_backspace() {
    if (input_pos > 0) {
        input_pos--;
        print_char('\b'); // Apaga o caractere na tela
        print_char(' ');  // Apaga o caractere
        print_char('\b'); // Retorna o cursor
    }
}

void handle_arrow_up() {
    if (history_index > 0) {
        history_index--;
        clear_input_buffer();
        strcpy(input_buffer, command_history[history_index]);
        input_pos = strlen(input_buffer);
        print_string(input_buffer);
    }
}

void handle_arrow_down() {
    if (history_index < history_pos - 1) {
        history_index++;
        clear_input_buffer();
        strcpy(input_buffer, command_history[history_index]);
        input_pos = strlen(input_buffer);
        print_string(input_buffer);
    }
}

void kernel_main() {
    const char *msg = "Digite um comando:\n";
    while (*msg) print_char(*msg++);

    unsigned char last_scancode = 0;

    while (1) {
        unsigned char scancode = read_scancode();

        if (scancode == last_scancode)
            continue;

        last_scancode = scancode;

        if (scancode & 0x80) {
            // tecla solta — zera last_scancode para permitir nova leitura
            last_scancode = 0;
        } else {
            // Filtra as teclas especiais (não adiciona ao buffer de entrada)
            if (scancode == 0x1C) {  // Enter
                handle_enter();
            } else if (scancode == 0x0E) {  // Backspace
                handle_backspace();
            } else if (scancode == 0x48) {  // Seta para cima
                handle_arrow_up();
            } else if (scancode == 0x50) {  // Seta para baixo
                handle_arrow_down();
            } else {
                // Adiciona caracteres imprimíveis ao buffer
                char c = scancode_to_ascii(scancode);
                if (c) {
                    if (input_pos < MAX_INPUT_SIZE - 1) {
                        input_buffer[input_pos++] = c;
                        print_char(c);  // Mostra o caractere na tela
                    }
                }
            }
        }
    }
}
