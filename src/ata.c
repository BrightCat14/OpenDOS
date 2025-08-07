#include "ata.h"
#include <string.h>
#include "io.h"
#include "os.h"

#define SECTOR_SIZE 512

ata_device_t devices[4];

static void ioa_wait() {
    for (int i = 0; i < 4; i++) {
        inb(0x80);
    }
}

int ata_identify(ata_device_t* dev, uint16_t* buffer) {
    outb(dev->io_base + ATA_REG_HDDEVSEL, dev->slave ? 0xB0 : 0xA0);
    ioa_wait();
    outb(dev->io_base + ATA_REG_SECCOUNT0, 0);
    outb(dev->io_base + ATA_REG_LBA0, 0);
    outb(dev->io_base + ATA_REG_LBA1, 0);
    outb(dev->io_base + ATA_REG_LBA2, 0);
    outb(dev->io_base + ATA_REG_COMMAND, ATA_CMD_IDENTIFY);

    if (inb(dev->io_base + ATA_REG_STATUS) == 0) return -1;

    for (int i = 0; i < 256; i++)
        buffer[i] = inw(dev->io_base + ATA_REG_DATA);

    for (int i = 0; i < 40; i += 2) {
        dev->model[i] = buffer[27 + i / 2] >> 8;
        dev->model[i + 1] = buffer[27 + i / 2] & 0xFF;
    }
    dev->model[40] = '\0';

    dev->exists = true;
    return 0;
}

int ata_read_sector(ata_device_t* dev, uint32_t lba, uint8_t* buffer) {
    outb(dev->io_base + ATA_REG_HDDEVSEL, 0xE0 | (dev->slave << 4) | ((lba >> 24) & 0x0F));
    outb(dev->io_base + ATA_REG_SECCOUNT0, 1);
    outb(dev->io_base + ATA_REG_LBA0, (uint8_t)(lba & 0xFF));
    outb(dev->io_base + ATA_REG_LBA1, (uint8_t)((lba >> 8) & 0xFF));
    outb(dev->io_base + ATA_REG_LBA2, (uint8_t)((lba >> 16) & 0xFF));
    outb(dev->io_base + ATA_REG_COMMAND, ATA_CMD_READ_SECTORS);

    for (int i = 0; i < SECTOR_SIZE / 2; i++) {
        ((uint16_t*)buffer)[i] = inw(dev->io_base + ATA_REG_DATA);
    }

    return 0;
}

int ata_write_sector(ata_device_t* dev, uint32_t lba, const uint8_t* buffer) {
    outb(dev->io_base + ATA_REG_HDDEVSEL, 0xE0 | (dev->slave << 4) | ((lba >> 24) & 0x0F));
    outb(dev->io_base + ATA_REG_SECCOUNT0, 1);
    outb(dev->io_base + ATA_REG_LBA0, (uint8_t)(lba & 0xFF));
    outb(dev->io_base + ATA_REG_LBA1, (uint8_t)((lba >> 8) & 0xFF));
    outb(dev->io_base + ATA_REG_LBA2, (uint8_t)((lba >> 16) & 0xFF));
    outb(dev->io_base + ATA_REG_COMMAND, ATA_CMD_WRITE_SECTORS);

    for (int i = 0; i < SECTOR_SIZE / 2; i++) {
        outw(dev->io_base + ATA_REG_DATA, ((uint16_t*)buffer)[i]);
    }

    return 0;
}

void ata_detect_all() {
    int idx = 0;
    for (int channel = 0; channel < 2; channel++) {
        for (int slave = 0; slave < 2; slave++) {
            ata_device_t* dev = &devices[idx];
            dev->io_base = (channel == 0) ? ATA_PRIMARY_IO : ATA_SECONDARY_IO;
            dev->ctrl_base = (channel == 0) ? ATA_PRIMARY_CTRL : ATA_SECONDARY_CTRL;
            dev->slave = slave;
            dev->exists = false;

            uint16_t identify_data[256];
            if (ata_identify(dev, identify_data) == 0) {}
            idx++;
        }
    }
}

void ata_print_devices() {
    for (int i = 0; i < 4; i++) {
        ata_device_t* dev = &devices[i];
        if (dev->exists) {
            k_printf("ATA Device %d: %s (IO: 0x%x, Slave: %d)\n", i, dev->model, dev->io_base, dev->slave);
        } else {
            k_printf("ATA Device %d: not found\n", i);
        }
    }
}

void ata_init() {
	ata_detect_all();
}
