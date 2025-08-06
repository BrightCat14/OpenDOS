#ifndef ATA_H
#define ATA_H
#define ATA_PRIMARY_CMD    0x1F0
#define ATA_PRIMARY_CTRL   0x3F6
#define ATA_PRIMARY_STATUS 0x1F7
#define ATA_PRIMARY_DATA   0x1F0
#define ATA_DATA        0x1F0
#define ATA_SECTOR_COUNT 0x1F2
#define ATA_LBA_LOW     0x1F3
#define ATA_LBA_MID     0x1F4
#define ATA_LBA_HIGH    0x1F5
#define ATA_DRIVE_SELECT 0x1F6
#define ATA_COMMAND     0x1F7
#define ATA_STATUS      0x1F7
#define ATA_CMD_WRITE_SECTORS 0x30
#define ATA_STATUS_BSY  0x80
#define ATA_STATUS_DRQ  0x08
#include <stdint.h>

void ata_wait();
int ata_identify(int drive, char* model_out);
void ata_read_sector(uint32_t lba, uint8_t* buffer);  
void ata_write_sector(uint32_t lba, const uint8_t* buf); 

#endif


