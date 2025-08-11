#include "fat12.h"
#include "ata.h"
#include <string.h>
#include <stdint.h>

#define SECTOR_SIZE 512
#define MAX_ENTRIES 224
#define CLUSTER_EOF 0xFF8

static uint8_t sector_buffer[SECTOR_SIZE];

static int read_sector(fat12_fs_t* fs, uint32_t lba, uint8_t* buf) {
    return ata_read_sector(fs->drive, fs->partition_start_lba + lba, buf);
}

static int write_sector(fat12_fs_t* fs, uint32_t lba, const uint8_t* buf) {
    return ata_write_sector(fs->drive, fs->partition_start_lba + lba, buf);
}

static uint32_t cluster_to_lba(fat12_fs_t* fs, uint16_t cluster) {
    return fs->data_start_lba + (cluster - 2) * fs->sectors_per_cluster;
}

static uint16_t read_fat_entry(fat12_fs_t* fs, uint16_t cluster) {
    uint32_t fat_offset = (cluster * 3) / 2;
    uint32_t fat_sector = fs->fat_start_lba + (fat_offset / SECTOR_SIZE);
    uint32_t offset = fat_offset % SECTOR_SIZE;

    if (read_sector(fs, fat_sector, sector_buffer) != 0)
        return 0xFFF;

    uint16_t value;
    if (cluster & 1)
        value = ((sector_buffer[offset] >> 4) | (sector_buffer[offset + 1] << 4)) & 0xFFF;
    else
        value = (sector_buffer[offset] | ((sector_buffer[offset + 1] & 0x0F) << 8)) & 0xFFF;
    return value;
}

static void write_fat_entry(fat12_fs_t* fs, uint16_t cluster, uint16_t value) {
    uint32_t fat_offset = (cluster * 3) / 2;
    uint32_t fat_sector = fs->fat_start_lba + (fat_offset / SECTOR_SIZE);
    uint32_t offset = fat_offset % SECTOR_SIZE;

    read_sector(fs, fat_sector, sector_buffer);
    if (cluster & 1) {
        sector_buffer[offset] = (sector_buffer[offset] & 0x0F) | ((value << 4) & 0xF0);
        sector_buffer[offset + 1] = value >> 4;
    } else {
        sector_buffer[offset] = value & 0xFF;
        sector_buffer[offset + 1] = (sector_buffer[offset + 1] & 0xF0) | ((value >> 8) & 0x0F);
    }
    write_sector(fs, fat_sector, sector_buffer);
}

static uint16_t find_free_cluster(fat12_fs_t* fs) {
    for (uint16_t i = 2; i < 0xFF7; i++) {
        if (read_fat_entry(fs, i) == 0x000)
            return i;
    }
    return 0;
}

static int strcasecmp(const char* s1, const char* s2) {
    while (*s1 && *s2) {
        char c1 = tolower((unsigned char)*s1);
        char c2 = tolower((unsigned char)*s2);
        if (c1 != c2)
            return c1 - c2;
        s1++; s2++;
    }
    return *s1 - *s2;
}

int fat12_mount(fat12_fs_t* fs, ata_device_t* drive) {
    uint8_t mbr[SECTOR_SIZE];
    if (ata_read_sector(drive, 0, mbr) != 0)
        return -1;
    uint32_t partition_start = *(uint32_t*)&mbr[446 + 8];

    if (partition_start == 0)
        return -1;

    fs->partition_start_lba = partition_start;

    if (ata_read_sector(drive, fs->partition_start_lba, sector_buffer) != 0)
        return -1;

    fs->drive = drive;
    fs->bytes_per_sector = *(uint16_t*)&sector_buffer[11];
    fs->sectors_per_cluster = sector_buffer[13];
    fs->reserved_sectors = *(uint16_t*)&sector_buffer[14];
    fs->fat_count = sector_buffer[16];
    fs->root_entries = *(uint16_t*)&sector_buffer[17];
    fs->total_sectors = *(uint16_t*)&sector_buffer[19];
    fs->sectors_per_fat = *(uint16_t*)&sector_buffer[22];

    fs->fat_start_lba = fs->partition_start_lba + fs->reserved_sectors;
    fs->root_dir_lba = fs->fat_start_lba + (fs->fat_count * fs->sectors_per_fat);
    fs->data_start_lba = fs->root_dir_lba + ((fs->root_entries * 32 + 511) / 512);

    return 0;
}

int fat12_list_dir(fat12_fs_t* fs, uint16_t cluster, fat12_file_t* out, int max) {
    int found = 0;
    uint32_t lba = cluster == 0 ? fs->root_dir_lba : cluster_to_lba(fs, cluster);
    int entries = cluster == 0 ? fs->root_entries : (fs->sectors_per_cluster * (fs->bytes_per_sector / 32));

    for (int s = 0; s < 14 && found < max; s++) {
        if (read_sector(fs, lba + s, sector_buffer) != 0)
            break;
        for (int i = 0; i < entries && found < max; i++) {
            uint8_t* e = &sector_buffer[i * 32];
            if (e[0] == 0x00 || e[0] == 0xE5) continue;
            if ((e[11] & 0x0F) == 0x0F) continue; // LFN entry
            fat12_file_t* f = &out[found++];
            memcpy(f->name, e, 8);
            f->name[8] = '.';
            memcpy(&f->name[9], e + 8, 3);
            f->name[12] = 0;
            for (int j = 10; j >= 0; j--) if (f->name[j] == ' ') f->name[j] = 0;
            f->cluster = *(uint16_t*)&e[26];
            f->size = *(uint32_t*)&e[28];
            f->is_dir = (e[11] & 0x10) ? 1 : 0;
        }
        if (cluster == 0) break;
        cluster = read_fat_entry(fs, cluster);
        if (cluster >= CLUSTER_EOF) break;
        lba = cluster_to_lba(fs, cluster);
    }
    return found;
}

int fat12_find(fat12_fs_t* fs, const char* path, fat12_file_t* out) {
    char token[13];
    const char* p = path;
    uint16_t current = 0;
    fat12_file_t list[32];

    while (*p) {
        int len = 0;
        while (*p && *p != '/' && len < 12) token[len++] = *p++;
        token[len] = 0;
        if (*p == '/') p++;

        int count = fat12_list_dir(fs, current, list, 32);
        int found = 0;
        for (int i = 0; i < count; i++) {
            if (strcasecmp(list[i].name, token) == 0) {
                *out = list[i];
                current = out->is_dir ? out->cluster : 0;
                found = 1;
                break;
            }
        }
        if (!found) return -1;
    }
    return 0;
}

int fat12_read_file(fat12_fs_t* fs, fat12_file_t* file, uint8_t* buf, uint32_t max) {
    uint16_t cluster = file->cluster;
    uint32_t total = 0;
    while (cluster < CLUSTER_EOF && total < max) {
        uint32_t lba = cluster_to_lba(fs, cluster);
        for (int i = 0; i < fs->sectors_per_cluster; i++) {
            if (total >= file->size || total >= max) break;
            if (read_sector(fs, lba + i, &buf[total]) != 0)
                return total;
            total += fs->bytes_per_sector;
        }
        cluster = read_fat_entry(fs, cluster);
    }
    return total;
}

int fat12_write_file(fat12_fs_t* fs, const char* path, const uint8_t* data, uint32_t size) {
    char path_copy[128];
    strncpy(path_copy, path, 127);
    path_copy[127] = 0;
    char* last = strrchr(path_copy, '/');
    char* filename = path_copy;
    if (last) {
        *last = 0;
        filename = last + 1;
    }

    fat12_file_t dir;
    if (last) {
        if (fat12_find(fs, path_copy, &dir) != 0) return -1;
    } else {
        dir.cluster = 0;
    }

    uint16_t first_cluster = find_free_cluster(fs);
    if (!first_cluster) return -2;

    uint16_t cluster = first_cluster;
    uint32_t written = 0;
    while (written < size) {
        uint32_t lba = cluster_to_lba(fs, cluster);
        for (int i = 0; i < fs->sectors_per_cluster && written < size; i++) {
            if (write_sector(fs, lba + i, &data[written]) != 0)
                return -4;
            written += fs->bytes_per_sector;
        }
        uint16_t next = (written < size) ? find_free_cluster(fs) : CLUSTER_EOF;
        write_fat_entry(fs, cluster, next);
        cluster = next;
    }

    uint32_t lba = dir.cluster == 0 ? fs->root_dir_lba : cluster_to_lba(fs, dir.cluster);
    for (int s = 0; s < 14; s++) {
        if (read_sector(fs, lba + s, sector_buffer) != 0)
            return -5;
        for (int i = 0; i < 16; i++) {
            uint8_t* e = &sector_buffer[i * 32];
            if (e[0] == 0x00 || e[0] == 0xE5) {
                memset(e, 0, 32);
                char name8[8] = { ' ' };
                char ext3[3] = { ' ' };
                const char* dot = strrchr(filename, '.');
                if (dot) {
                    size_t nlen = dot - filename;
                    if (nlen > 8) nlen = 8;
                    memcpy(name8, filename, nlen);
                    size_t elen = strlen(dot + 1);
                    if (elen > 3) elen = 3;
                    memcpy(ext3, dot + 1, elen);
                } else {
                    size_t nlen = strlen(filename);
                    if (nlen > 8) nlen = 8;
                    memcpy(name8, filename, nlen);
                }
                memcpy(e, name8, 8);
                memcpy(e + 8, ext3, 3);

                e[11] = 0x20;
                *(uint16_t*)&e[26] = first_cluster;
                *(uint32_t*)&e[28] = size;
                write_sector(fs, lba + s, sector_buffer);
                return 0;
            }
        }
    }

    return -3;
}
