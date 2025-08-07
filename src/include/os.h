#pragma once
#include "stdint.h"

extern uint16_t cursor_pos;
extern uint16_t input_pos;

void k_clear(void);
void k_print(const char *str);
void k_printf(const char *fmt, ...);
void k_putc(char c, uint8_t color);
