#pragma once
#include "stdint.h"

uint16_t cursor_pos = 0;
uint16_t input_pos = 0;

void k_clear(void);
void k_print(const char *str);
void k_printf(const char *fmt, ...);
void k_putc(char c, uint8_t color);
