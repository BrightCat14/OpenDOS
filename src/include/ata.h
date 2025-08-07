
#ifndef ATA_H
#define ATA_H

#include <stdint.h>
#include <stdbool.h>

#define ATA_PRIMARY_IO      0x1F0
#define ATA_PRIMARY_CTRL    0x3F6
#define ATA_SECONDARY_IO    0x170
#define ATA_SECONDARY_CTRL  0x376

#define ATA_REG_DATA        0x00
#define ATA_REG_ERROR       0x01
#define ATA_REG_FEATURES    0x01
#define ATA_REG_SECCOUNT0   0x02
#define ATA_REG_LBA0        0x03
#define ATA_REG_LBA1        0x04
#define ATA_REG_LBA2        0x05
#define ATA_REG_HDDEVSEL    0x06
#define ATA_REG_COMMAND     0x07
#define ATA_REG_STATUS      0x07
#define ATA_REG_CONTROL     0x206

#define ATA_CMD_READ_SECTORS     0x20
#define ATA_CMD_WRITE_SECTORS    0x30
#define ATA_CMD_IDENTIFY         0xEC

typedef struct {
    uint16_t io_base;
    uint16_t ctrl_base;
    uint8_t slave;
    bool exists;
    char model[41];
} ata_device_t;

extern ata_device_t devices[4];

void ata_init();
void ata_detect_all();
int ata_read_sector(ata_device_t* dev, uint32_t lba, uint8_t* buffer);
int ata_write_sector(ata_device_t* dev, uint32_t lba, const uint8_t* buffer);
int ata_identify(ata_device_t* dev, uint16_t* buffer);
void ata_print_devices();

#endif
