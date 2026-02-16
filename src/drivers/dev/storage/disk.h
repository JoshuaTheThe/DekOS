/**
 * DEPRECATED
 */

#ifndef DISK_H
#define DISK_H

#include <utils.h>
#include <io.h>

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

/* __attribute__((deprecated)) */ void cdInit(int *port, bool *slave);
/* __attribute__((deprecated)) */ int cdDetectMedia(uint16_t port, bool slave);
/* __attribute__((deprecated)) */ int cdDetect(uint16_t port, bool slave);
/* __attribute__((deprecated)) */ int cdMediaPresent(uint16_t port, bool slave);
/* __attribute__((deprecated)) */ int cdRead(uint16_t port, bool slave, uint32_t lba, uint32_t sectors, uint16_t *buffer);

#endif
