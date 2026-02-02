#ifndef FAT_H
#define FAT_H

#include <utils.h>
#include <drivers/serial.h>
#include <drivers/storage.h>
#include <memory/string.h>

#define FAT_BOOT_SECTOR_SIGN (0xAA55)

#define FAT_EXT_BOOT_RECORD_16_SIGN_A (0x28)
#define FAT_EXT_BOOT_RECORD_16_SIGN_B (0x29)

#define FAT_INFO_MAGIC_A (0x41615252)
#define FAT_INFO_MAGIC_B (0x61417272)
#define FAT_INFO_MAGIC_C (0xAA550000)

#define FAT_READ_ONLY (0x01)
#define FAT_HIDDEN (0x02)
#define FAT_SYSTEM (0x04)
#define FAT_VOLUME_ID (0x08)
#define FAT_DIRECTORY (0x10)
#define FAT_ARCHIVE (0x20)
// #define FAT_LONG_FILE_NAME (FAT_READ_ONLY|FAT_HIDDEN|FAT_SYSTEM|FAT_VOLUME_ID|FAT_DIRECTORY|FAT_ARCHIVE)

#define FAT_UNUSED (0xE5)

typedef WORD SECTOR;

typedef enum
{
        FAT_UNKNOWN,
        FAT_12,
        FAT_16,
        FAT_32,
} FATType;

typedef struct __attribute__((__packed__))
{
        BYTE __jmp_nop[3]; /* e.g. JMP _Start followed by a NOP instruction */
        BYTE OEMIdentifier[8];
        WORD BytesPerSector;
        BYTE SectorsPerCluster;
        SECTOR ReservedSectors; /* including boot record sectors */
        BYTE FATCount;
        WORD RootDirectoryEntries;
        SECTOR TotalSectorsInVolume; /* IF 0 THEN check 0x20 instead */
        BYTE MediaDescType;
        SECTOR SectorsPerFAT; /* FAT12/16 only. */
        SECTOR SectorsPerTrack;
        WORD HeadsCount;
        DWORD HiddenSectors;
        DWORD ExtendedSectorCount; /* offset 0x20 */
} FATBiosParameterBlock;

typedef struct __attribute__((__packed__))
{
        BYTE DriveNum;
        BYTE Reserved;
        BYTE Signature;
        DWORD VolumeID;
        BYTE VolumeLabel[11];
        BYTE SystemIdentifier[8];
        BYTE BootCode[448];
} FATExtBootRecord_16;

typedef struct __attribute__((__packed__))
{
        DWORD SectorsPerFAT;
        WORD Flags;
        WORD FATVersion;
        DWORD RootCluster;
        SECTOR FSInfo;
        SECTOR BackupBootSector;
        BYTE Reserved[12];
        BYTE DriveNum;
        BYTE Reserved2;
        BYTE Signature;
        DWORD VolumeID;
        BYTE VolumeLabel[11];
        BYTE SystemIdentifier[8]; /* Always "FAT32  " */
        BYTE BootCode[420];
} FATExtBootRecord_32;

typedef struct __attribute__((__packed__))
{
        DWORD SignatureA;
        BYTE Reserved[480];
        DWORD SignatureB;
        DWORD LastFreeClusterCount;
        DWORD FirstAvailableCluster;
        BYTE Reserved2[12];
        DWORD SignatureC;
} FATInfo_32;

typedef struct __attribute__((__packed__))
{
        FATBiosParameterBlock bpb;

        union
        {
                FATExtBootRecord_16 fat16;
                FATExtBootRecord_32 fat32;
        } as;

        WORD Magic; /* 0xAA55 */
} FATBootSector;

typedef struct __attribute__((__packed__))
{
        BYTE Hour : 5;
        BYTE Minutes : 6;
        BYTE Seconds : 5; /* Multiply By 2 */
} FATTime;

typedef struct __attribute__((__packed__))
{
        BYTE Year : 7;
        BYTE Month : 4;
        BYTE Day : 5;
} FATDate;

typedef struct __attribute__((__packed__))
{
        BYTE Name[8], Ext[3];
        BYTE Flags;
        BYTE Reserved;
        BYTE __time_ignore; /* we do not care for the purposes of this OS */
        WORD Time;
        WORD Date;
        WORD LastAccessedDate;
        WORD EntryFirstClusterHigh;
        WORD LastModifiedTime;
        WORD LastModifiedDate;
        WORD EntryFirstClusterLow;
        DWORD Size; /* Bytes */
} FATDirectory;

typedef struct __attribute__((__packed__))
{
        FATDirectory Dir;
        DWORD Cluster;
        DWORD Offset;
        DWORD Index;
        bool Found;
} FATFileLocation;

typedef struct __attribute__((__packed__))
{
        /* TODO - we dont care for now*/
} FATLongFileName;

FATBootSector FatReadBootSector(DRIVE *Drive);
DWORD FatTotalSectors(FATBootSector *Bt);
DWORD FatSize(FATBootSector *Bt);
DWORD FatRootDirSize(FATBootSector *Bt);
DWORD FatFirstData(FATBootSector *Bt);
DWORD FatFirstFat(FATBootSector *Bt);
DWORD FatDataCount(FATBootSector *Bt);
DWORD FatClusterCount(FATBootSector *Bt);
FATType FatIdentify(FATBootSector *Bt);
DWORD FatFirstRoot(FATBootSector *Bt);
DWORD FatRootCluster(FATBootSector *Bt);
DWORD FatFirstSectorForCluster(FATBootSector *Bt, DWORD Cluster);
void *FatRead(BYTE Name[11], BYTE Ext[3], FATBootSector *bt, DRIVE *Drive, FATDirectory *Parent);
void FatConvert83(const char *path, char *NameOut, char *ExtOut);
FATFileLocation FatLocateInDir(BYTE Name[11], BYTE Ext[3], FATBootSector *bt, DRIVE *Drive, FATDirectory *Parent);
void FatConvertPaddedToNull(char *X, size_t L);

void FatTest(DRIVE *Drive);

#endif
