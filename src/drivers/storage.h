#ifndef STORAGE_H
#define STORAGE_H

#include <utils.h>
#include <pci/pci.h>
#include <drivers/serial.h>
#include <drivers/ide.h>

typedef struct DRIVE
{
        uint8_t BufferA[4096];
        uint8_t BufferB[4096];
        uint32_t DriveInfo[64];
        void (*ReadData)(struct DRIVE *Drive);
        void (*WriteData)(struct DRIVE *Drive);
        void *(*ReadFile)(struct DRIVE *Drive, const char *Path);
        void (*WriteFile)(struct DRIVE *Drive, const char *Path);
        size_t LBA, PCIDevIndex;
        bool Valid, Present;

        char Model[41];
        uint8_t Type;
        uint8_t Channel;
        uint8_t DriveNum;
        uint64_t Size;
} DRIVE;

void SMChange(size_t New);
void *SMWrite(size_t LBA, void *Buf, size_t BufSize);
void *SMRead(size_t LBA);
void SMInit(void);
DRIVE *SMGetDrive(void);
DRIVE *SMPopDrive(void);
DRIVE *SMPushDrive(DRIVE *Drive);
void *SMWriteTo(size_t LBA, void *Buf, size_t BufSize, DRIVE *Drive);
void *SMReadFrom(size_t LBA, DRIVE *Drive);

extern IDEDriver_t IDEState;

#endif
