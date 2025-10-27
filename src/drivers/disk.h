#ifndef DISK_H
#define DISK_H

#include <stdint.h>
#include <stdbool.h>
#include <io.h>
#include <tty/output/output.h>

#define DATA 0
#define ERROR_R 1
#define SECTOR_COUNT 2
#define LBA_LOW 3
#define LBA_MID 4
#define LBA_HIGH 5
#define DRIVE_SELECT 6
#define COMMAND_REGISTER 7
#define STATUS 7
#define ALTERNATE_STATUS 0
#define CONTROL 0x206
#define ALTERNATE_STATUS 0

extern void cd_drive_init(int *port, bool *slave);
extern int detect_cdrom(uint16_t port, bool slave);
extern int cdrom_detect(uint16_t port, bool slave);
extern int cdrom_media_present(uint16_t port, bool slave);
extern int read_cdrom(uint16_t port, bool slave, uint32_t lba, uint32_t sectors, uint16_t *buffer);

#endif
