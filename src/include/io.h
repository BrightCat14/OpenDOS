#pragma once

// dangerous
uint8_t inb(uint16_t port);
void outb(uint16_t port, uint8_t val);
uint16_t inw(uint16_t port);
void insw(uint16_t port, void* addr, int count);
void outw(uint16_t port, uint16_t val);

// safer
void io_wait();
