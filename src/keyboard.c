#include <stdint.h>
#include "io.h"
#include "os.h"
#include "colors.h"
#include "keyboard.h"
#include "os.h"

static char input_buffer[128];
extern uint16_t cursor_pos;  
extern uint16_t input_pos;   
static uint8_t keyboard_leds = 0;
static uint8_t shift_pressed = 0;
static uint8_t caps_lock = 0;
#define KEYBOARD_PORT 0x60
#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64

static const char kbd_us[128] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
    '*', 0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    '7', '8', '9', '-', '4', '5', '6', '+', '1', '2', '3', '0', '.'
};

void update_cursor() {
    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t)(cursor_pos & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t)((cursor_pos >> 8) & 0xFF));
}

void keyboard_handler() {
    static uint8_t extended = 0;
    uint8_t status = inb(KEYBOARD_STATUS_PORT);
    
    if (!(status & 0x01)) return;

    uint8_t scancode = inb(KEYBOARD_DATA_PORT);
    
    if (scancode == 0xE0) {
        extended = 1;
        return;
    }
    

    if (extended && scancode == 0x48) {
        extended = 0;
        return;
    }
    
    if (extended && scancode == 0x50) {
        extended = 0;
        return;
    }

    if (scancode & 0x80) {
        uint8_t released_scancode = scancode & 0x7F;
        
        if (released_scancode == 0x2A || released_scancode == 0x36) {
            shift_pressed = 0;
        }
        return;
    }
    
    if (scancode == 0x2A || scancode == 0x36) {
        shift_pressed = 1;
        return;
    }
    
    if (scancode == 0x3A) {
        caps_lock = !caps_lock;
        
        keyboard_leds ^= 0x04;
        outb(0x60, 0xED);
        while ((inb(0x64) & 0x02) != 0);
        outb(0x60, keyboard_leds);
        return;
    }
    
    if (scancode == 0x0E) {
        if (input_pos > 0) {
            input_pos--;
            cursor_pos--;
            k_putc(' ', WHITE_ON_BLACK);
            cursor_pos--;
            update_cursor();
            input_buffer[input_pos] = '\0';
        }
        return;
    }
    
    if (scancode == 0x1C) {
        k_putc('\n', WHITE_ON_BLACK);
        // process_command(input_buffer);
        input_pos = 0;
        input_buffer[0] = '\0';
        k_print("> ");
        return;
    }
    
    if (scancode < sizeof(kbd_us) && kbd_us[scancode] != 0) {
        char c = kbd_us[scancode];
        
        int uppercase = shift_pressed ^ caps_lock;
        
        if (c >= 'a' && c <= 'z' && uppercase) {
            c -= 32;
        }
        else if (shift_pressed) {
            switch (c) {
                case '1': c = '!'; break;
                case '2': c = '@'; break;
                case '3': c = '#'; break;
                case '4': c = '$'; break;
                case '5': c = '%'; break;
                case '6': c = '^'; break;
                case '7': c = '&'; break;
                case '8': c = '*'; break;
                case '9': c = '('; break;
                case '0': c = ')'; break;
                case '-': c = '_'; break;
                case '=': c = '+'; break;
                case '[': c = '{'; break;
                case ']': c = '}'; break;
                case ';': c = ':'; break;
                case '\'': c = '"'; break;
                case '`': c = '~'; break;
                case '\\': c = '|'; break;
                case ',': c = '<'; break;
                case '.': c = '>'; break;
                case '/': c = '?'; break;
            }
        }
        
        if (input_pos < sizeof(input_buffer) - 1) {
            input_buffer[input_pos++] = c;
            input_buffer[input_pos] = '\0';
            k_putc(c, WHITE_ON_BLACK);
        }
    }
    
    extended = 0;
}

void init_keyboard() {
    while ((inb(0x64) & 0x01)) {
        inb(0x60);
    }
    
    outb(0x60, 0xED);
    while ((inb(0x64) & 0x02) != 0);
    outb(0x60, keyboard_leds);
}
