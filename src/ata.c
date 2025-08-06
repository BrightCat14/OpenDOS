#include "ata.h"
#include "io.h"
#include <stdint.h>

void ata_wait() {
    while (inb(0x1F7) & 0x80); 
    while (!(inb(0x1F7) & 0x08)); 
}

int ata_identify(int drive, char* model_out) {
    uint8_t status;

    outb(0x1F6, 0xA0 | (drive << 4));
    io_wait();

    outb(0x1F1, 0);
    outb(0x1F2, 0);
    outb(0x1F3, 0);
    outb(0x1F4, 0);
    outb(0x1F5, 0);

    outb(0x1F7, 0xEC);
    io_wait();

    status = inb(0x1F7);
    if (status == 0) return 0; 

    while ((status & 0x80) != 0) {
        status = inb(0x1F7);
    }
    if (status == 0) return 0;

    uint16_t identify_data[256];
    for (int i = 0; i < 256; i++) {
        identify_data[i] = inw(0x1F0);
    }

    for (int i = 0; i < 20; i++) {
        char c1 = (identify_data[27 + i] >> 8) & 0xFF;
        char c2 = identify_data[27 + i] & 0xFF;
        model_out[i * 2] = c1;
        model_out[i * 2 + 1] = c2;
    }
    model_out[40] = 0;

    return 1;
}


void ata_write_sector(uint32_t lba, const uint8_t* buf) {
    while (inb(ATA_STATUS) & ATA_STATUS_BSY);

    outb(ATA_DRIVE_SELECT, 0xE0 | ((lba >> 24) & 0x0F));
    outb(ATA_SECTOR_COUNT, 1); 
    outb(ATA_LBA_LOW,  (uint8_t)(lba & 0xFF));
    outb(ATA_LBA_MID,  (uint8_t)((lba >> 8) & 0xFF));
    outb(ATA_LBA_HIGH, (uint8_t)((lba >> 16) & 0xFF));

    outb(ATA_COMMAND, ATA_CMD_WRITE_SECTORS);

    while (!(inb(ATA_STATUS) & ATA_STATUS_DRQ));

    for (int i = 0; i < 256; i++) {
        uint16_t word = ((uint16_t)buf[i * 2 + 1] << 8) | buf[i * 2];
        outw(ATA_DATA, word);
    }

    while (inb(ATA_STATUS) & ATA_STATUS_BSY);
}

void ata_read_sector(uint32_t lba, uint8_t* buffer) {
    outb(0x1F6, 0xE0 | ((lba >> 24) & 0x0F)); 
    outb(0x1F2, 1);                       
    outb(0x1F3, lba & 0xFF);               
    outb(0x1F4, (lba >> 8) & 0xFF);        
    outb(0x1F5, (lba >> 16) & 0xFF);      
    outb(0x1F7, 0x20);                      

    ata_wait();
    insw(0x1F0, buffer, 256);             
}

