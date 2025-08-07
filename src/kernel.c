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
    for(uint16_t i = 0; i < 80*25; i++) {
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
        cursor_pos = (cursor_pos / 80 + 1) * 80;
    } else {
        vidmem[cursor_pos*2] = c;
        vidmem[cursor_pos*2+1] = color;
        cursor_pos++;
    }
    
    if(cursor_pos >= 80*25) {
        k_clear();
    }
    update_cursor();
}

void k_print(const char *str) {
    while(*str) k_putc(*str++, WHITE_ON_BLACK);
}

void k_printf(const char* fmt, ...) {
    uint32_t* args = (uint32_t*)&fmt + 1;
    
    while (*fmt) {
        if (*fmt == '%') {
            fmt++;
            switch (*fmt) {
                case 's': {
                    char* s = *(char**)args;
                    args++;
                    k_print(s);
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
                    k_print(p);
                    break;
                }
                case 'c': {
                    char c = *(char*)args;
                    args++;
                    k_putc(c, WHITE_ON_BLACK);
                    break;
                }
                case '%': {
                    k_putc('%', WHITE_ON_BLACK);
                    break;
                }
            }
            fmt++;
        } else {
            k_putc(*fmt++, WHITE_ON_BLACK);
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
	
	// beep
	speaker_beep(600); 
	delay_ms(5000);
	speaker_off();
}

void k_main(void) {
    k_init();

	fat12_file_t files[32];
	fat12_file_t dir;
	if (fat12_find(&fs, "OPENDOS", &dir) == 0 && dir.is_dir) { // fix fat12 driver please
		int count = fat12_list_dir(&fs, dir.cluster, files, 32);
		for (int i = 0; i < count; i++) {
			k_printf("%s (%u bytes)\n", files[i].name, files[i].size);
		}
	}

    k_clear();
    k_printf("OpenDOS Kernel %s\n", version);
    sysinfo();
    ata_print_devices();
    k_print("Type 'help' for commands\n> ");
    
    while(1) {
        keyboard_handler();
        __asm__("pause");
    }
}
