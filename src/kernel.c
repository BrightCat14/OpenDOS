// kernel.c

#include <stdint.h>
#include <colors.h>
#include <keyboard.h>
#include <ata.h>
#include <fat12.h>
#include "speaker.h"

// defines
#define VIDEO_MEMORY ((char*)0xB8000)
#define WIDTH 80
#define HEIGHT 25
static char version[] = "v0.0.1";
fat12_fs_t fs;
uint16_t cursor_pos = 0;
uint16_t input_pos = 0;

void k_clear(void) {
    char *vidmem = VIDEO_MEMORY;
    for(uint16_t i = 0; i < WIDTH * HEIGHT; i++) {
        vidmem[i*2] = ' ';
        vidmem[i*2+1] = WHITE_ON_BLACK;
    }
    cursor_pos = 0;
    input_pos = 0;
    update_cursor();
}

void k_putc(char c, uint8_t color) {
    char *vidmem = VIDEO_MEMORY;
    
    if(c == '\n') {
        cursor_pos = (cursor_pos / WIDTH + 1) * WIDTH;
    } else {
        vidmem[cursor_pos*2] = c;
        vidmem[cursor_pos*2+1] = color;
        cursor_pos++;
    }
    
    if(cursor_pos >= WIDTH * HEIGHT) {
        k_clear();
    }
    update_cursor();
}

void k_print(const char *str) {
    while(*str) k_putc(*str++, WHITE_ON_BLACK);
}

void k_printf(const char* fmt, ...) {
    uint32_t* args = (uint32_t*)&fmt + 1;
    uint8_t color = WHITE_ON_BLACK; // active color

    while (*fmt) {
        // ANSI escape sequence handling
        if (*fmt == '\x1b' && fmt[1] == '[') {
            fmt += 2; // skip ESC[

            int code = 0;
            while (*fmt >= '0' && *fmt <= '9') {
                code = code * 10 + (*fmt - '0');
                fmt++;
            }

            if (*fmt == 'm') {
                fmt++; // skip 'm'

                // Reset
                if (code == 0) {
                    color = WHITE_ON_BLACK;
                }
                // Foreground colors
                else if (code >= 30 && code <= 37) {
                    uint8_t fg;
                    switch (code) {
                        case 30: fg = VGA_BLACK; break;
                        case 31: fg = VGA_RED; break;
                        case 32: fg = VGA_GREEN; break;
                        case 33: fg = VGA_YELLOW; break;
                        case 34: fg = VGA_BLUE; break;
                        case 35: fg = VGA_MAGENTA; break;
                        case 36: fg = VGA_CYAN; break;
                        case 37: fg = VGA_WHITE; break;
                        default: fg = VGA_WHITE; break;
                    }
                    color = (color & 0xF0) | fg;
                }
                continue; // skip normal printing for escape sequences
            }
        }
        // printf-like parsing
        else if (*fmt == '%') {
            fmt++;
            switch (*fmt) {
                case 's': {
                    char* s = *(char**)args;
                    args++;
                    while (*s) k_putc(*s++, color);
                    break;
                }
                case 'd': {
                    int num = *(int*)args;
                    args++;
                    char buf[16];
                    char* p = buf + sizeof(buf) - 1;
                    *p = '\0';
                    
                    if (num == 0) {
                        *--p = '0';
                    } else {
                        int is_neg = 0;
                        if (num < 0) {
                            is_neg = 1;
                            num = -num;
                        }
                        while (num > 0) {
                            *--p = '0' + (num % 10);
                            num /= 10;
                        }
                        if (is_neg) *--p = '-';
                    }
                    while (*p) k_putc(*p++, color);
                    break;
                }
                case 'c': {
                    char c = *(char*)args;
                    args++;
                    k_putc(c, color);
                    break;
                }
                case '%': {
                    k_putc('%', color);
                    break;
                }
            }
            fmt++;
        } else {
            k_putc(*fmt++, color);
        }
    }
}

void sysinfo() {
    k_print("\nSystem Information:\n");
    k_print("------------------\n");
    k_printf("OS Version: OpenDOS %s\n", version);
}

void k_init(void) {
    init_keyboard();
    ata_init();

    if (fat12_mount(&fs, 0) != 0) {
        k_printf("FAT12 mount failed!\n");
        return;
    }
    
    speaker_beep(600); 
    delay_ms(5000);
    speaker_off();
}

void k_main(void) {
    k_init();

    fat12_file_t files[32];
    fat12_file_t dir;
    if (fat12_find(&fs, "OPENDOS", &dir) == 0 && dir.is_dir) {
        int count = fat12_list_dir(&fs, dir.cluster, files, 32);
        for (int i = 0; i < count; i++) {
            k_printf("%s (%u bytes)\n", files[i].name, files[i].size);
        }
    }

    k_clear();
    k_printf("OpenDOS Kernel %s\n", version);
    sysinfo();
    ata_print_devices();

    k_printf("ANSI Color Test:");
    k_printf("\x1b[30mBlack   \x1b[0m\n");
    k_printf("\x1b[31mRed     \x1b[0m\n");
    k_printf("\x1b[32mGreen   \x1b[0m\n");
    k_printf("\x1b[33mYellow  \x1b[0m\n");
    k_printf("\x1b[34mBlue    \x1b[0m\n");
    k_printf("\x1b[35mMagenta \x1b[0m\n");
    k_printf("\x1b[36mCyan    \x1b[0m\n");
    k_printf("\x1b[37mWhite   \x1b[0m\n");

    k_print("Type 'help' for commands\n> ");
    
    while(1) {
        keyboard_handler();
        __asm__("pause");
    }
}
