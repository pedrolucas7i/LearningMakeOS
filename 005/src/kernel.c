#include <stddef.h> // Para o size_t

#define VIDEO_MEMORY (char *)0xB8000
#define PORT_KEYBOARD 0x60
#define MAX_INPUT_SIZE 128
#define MAX_HISTORY_SIZE 10
#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 25

typedef unsigned int size_t;

static int cursor_x = 0;
static int cursor_y = 0;
static char input_buffer[MAX_INPUT_SIZE];
static int input_pos = 0;
static char command_history[MAX_HISTORY_SIZE][MAX_INPUT_SIZE];
static int history_pos = 0;
static int history_index = 0;

// Prototipagem das funções que estão sendo usadas antes de sua definição
void print_char(char c);
void print_string(const char *str);
int strcmp(const char *str1, const char *str2);
char *strcpy(char *dest, const char *src);
size_t strlen(const char *str);
void clear_input_buffer();
void handle_enter();
void clear_screen();
unsigned char read_scancode();
char scancode_to_ascii(unsigned char sc);
void handle_backspace();
void handle_arrow_up();
void handle_arrow_down();
void draw_prompt();
void handle_newline(); // Função para tratar o \n corretamente

void print_char(char c) {
    volatile char *video = VIDEO_MEMORY;
    video[(cursor_y * SCREEN_WIDTH + cursor_x) * 2] = c;
    video[(cursor_y * SCREEN_WIDTH + cursor_x) * 2 + 1] = 0x0F; // Branco sobre preto

    cursor_x++;

    if (cursor_x >= SCREEN_WIDTH) {
        cursor_x = 0;
        cursor_y++;
        if (cursor_y >= SCREEN_HEIGHT) {
            for (int i = 1; i < SCREEN_HEIGHT; i++) {
                for (int j = 0; j < SCREEN_WIDTH; j++) {
                    video[((i - 1) * SCREEN_WIDTH + j) * 2] = video[(i * SCREEN_WIDTH + j) * 2];
                    video[((i - 1) * SCREEN_WIDTH + j) * 2 + 1] = video[(i * SCREEN_WIDTH + j) * 2 + 1];
                }
            }
            for (int i = 0; i < SCREEN_WIDTH; i++) {
                video[((SCREEN_HEIGHT - 1) * SCREEN_WIDTH + i) * 2] = ' ';
                video[((SCREEN_HEIGHT - 1) * SCREEN_WIDTH + i) * 2 + 1] = 0x0F;
            }
            cursor_y = SCREEN_HEIGHT - 1;
        }
    }
}

void print_string(const char *str) {
    while (*str) {
        if (*str == '\n') {
            handle_newline();
        } else {
            print_char(*str);
        }
        str++;
    }
}

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

void draw_prompt() {
    print_string("\nMyOS> ");
}

void handle_enter() {
    input_buffer[input_pos] = '\0'; // Garantir que a string termine
    print_string("\nComando executado: ");
    print_string(input_buffer);

    // Adiciona ao histórico de comandos
    if (history_pos < MAX_HISTORY_SIZE) {
        strcpy(command_history[history_pos], input_buffer);
        history_pos++;
    }

    if (strcmp(input_buffer, "help") == 0) {
        print_string("\nComandos disponíveis:\n");
        print_string("help - Exibe esta lista de comandos\n");
        print_string("clear - Limpa a tela\n");
    } else if (strcmp(input_buffer, "clear") == 0) {
        clear_screen();
    } else {
        print_string("\nComando desconhecido: ");
        print_string(input_buffer);
    }

    clear_input_buffer();
    cursor_x = 0;
    cursor_y++;
    draw_prompt();
}

void clear_screen() {
    for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++) {
        *((volatile char*)VIDEO_MEMORY + i * 2) = ' ';
    }
    cursor_x = 0;
    cursor_y = 0;
    draw_prompt();
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

void handle_backspace() {
    if (input_pos > 0) {
        input_pos--;
        input_buffer[input_pos] = '\0';
        cursor_x--;
        print_char(' ');  // Apaga o caractere da tela
        cursor_x--;
        print_char(' ');  // Apaga o espaço que fica no lugar
    }
}

void handle_arrow_up() {
    if (history_index > 0) {
        history_index--;
        strcpy(input_buffer, command_history[history_index]);
        input_pos = strlen(input_buffer);
        clear_screen();
        draw_prompt();
        print_string(input_buffer);
    }
}

void handle_arrow_down() {
    if (history_index < history_pos) {
        history_index++;
        strcpy(input_buffer, command_history[history_index]);
        input_pos = strlen(input_buffer);
        clear_screen();
        draw_prompt();
        print_string(input_buffer);
    }
}

void handle_newline() {
    cursor_x = 0;
    cursor_y++;
    if (cursor_y >= SCREEN_HEIGHT) {
        for (int i = 1; i < SCREEN_HEIGHT; i++) {
            for (int j = 0; j < SCREEN_WIDTH; j++) {
                *((volatile char*)VIDEO_MEMORY + ((i - 1) * SCREEN_WIDTH + j) * 2) = 
                *((volatile char*)VIDEO_MEMORY + ((i * SCREEN_WIDTH + j) * 2));
            }
        }
        for (int i = 0; i < SCREEN_WIDTH; i++) {
            *((volatile char*)VIDEO_MEMORY + ((SCREEN_HEIGHT - 1) * SCREEN_WIDTH + i) * 2) = ' ';
        }
        cursor_y = SCREEN_HEIGHT - 1;
    }
    print_char(' '); // Para garantir que o caractere após o \n é removido
    cursor_x = 0;
}

void kernel_main() {
    draw_prompt();  // Exibe o prompt logo ao começar

    unsigned char last_scancode = 0;

    while (1) {
        unsigned char scancode = read_scancode();

        if (scancode == last_scancode)
            continue;

        last_scancode = scancode;

        if (scancode & 0x80) {
            // tecla solta
            last_scancode = 0;
        } else {
            // Filtra as teclas especiais
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
