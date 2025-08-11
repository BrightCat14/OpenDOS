#ifndef FAT12_H
#define FAT12_H

#include <stdint.h>
#include <ata.h>

typedef struct {
    ata_device_t* drive;
    uint16_t bytes_per_sector;
    uint8_t sectors_per_cluster;
    uint16_t reserved_sectors;
    uint8_t fat_count;
    uint16_t root_entries;
    uint16_t total_sectors;
    uint16_t sectors_per_fat;
    uint32_t fat_start_lba;
    uint32_t root_dir_lba;
    uint32_t data_start_lba;

    uint32_t partition_start_lba;
} fat12_fs_t;

typedef struct {
    char name[13];
    uint16_t cluster;
    uint32_t size;
    uint8_t is_dir;
} fat12_file_t;

int fat12_mount(fat12_fs_t* fs, ata_device_t* drive);

int fat12_list_dir(fat12_fs_t* fs, uint16_t cluster, fat12_file_t* out, int max);

int fat12_find(fat12_fs_t* fs, const char* path, fat12_file_t* out_file);

int fat12_read_file(fat12_fs_t* fs, fat12_file_t* file, uint8_t* buf, uint32_t max_size);

int fat12_write_file(fat12_fs_t* fs, const char* path, const uint8_t* data, uint32_t size);

#endif
