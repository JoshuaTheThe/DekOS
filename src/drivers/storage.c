#include <drivers/storage.h>
#include <drivers/fat.h>
#include <memory/string.h>
#include <math.h>

DRIVE Drives[64]; /* 00 - 63 */
DRIVE *CurrentDrive = NULL;
size_t DriveCount;

DRIVE *DriveStack[64];
size_t DriveStackSP;

void IDEReadData(DRIVE *Self)
{
        IDEReadSectors(Self->DriveNum, 1, Self->LBA, 0x10, (unsigned int)Self->BufferA);
}

void IDEWriteData(DRIVE *Self)
{
        IDEWriteSectors(Self->DriveNum, 1, Self->LBA, 0x10, (unsigned int)Self->BufferA);
}

void *IDEReadFile(DRIVE *Self, const char *Path)
{
        FATBootSector bt = FatReadBootSector(Self);

        FATFileLocation Loc = {0};
        FATDirectory CurrentDir = {0};
        FATDirectory *pCurrentDir = NULL;

        char PathCopy[256];
        strncpy(PathCopy, Path, sizeof(PathCopy));
        PathCopy[sizeof(PathCopy) - 1] = 0;

        char *segment = strtok(PathCopy, "/");
        char *lastSegment = NULL;
        char Name[9] = {0}, Extension[4] = {0};

        while (segment)
        {
                lastSegment = segment;
                FatConvert83(segment, Name, Extension);
                Loc = FatLocateInDir((BYTE *)Name, (BYTE *)Extension, &bt, Self, pCurrentDir);
                if (!Loc.Found)
                        return NULL;

                if (Loc.Dir.Flags & FAT_DIRECTORY)
                {
                        CurrentDir = Loc.Dir;
                        pCurrentDir = &CurrentDir;
                }

                segment = strtok(NULL, "/");
        }

        if (!lastSegment)
        {
                return NULL;
        }

        return FatRead((BYTE *)Name, (BYTE *)Extension, &bt, Self, pCurrentDir);
}

DWORD IDEFileSize(DRIVE *Self, const char *Path)
{
        FATBootSector bt = FatReadBootSector(Self);

        FATFileLocation Loc = {0};
        FATDirectory CurrentDir = {0};
        FATDirectory *pCurrentDir = NULL;

        char PathCopy[256];
        strncpy(PathCopy, Path, sizeof(PathCopy));
        PathCopy[sizeof(PathCopy) - 1] = 0;

        char *segment = strtok(PathCopy, "/");
        char *lastSegment = NULL;
        char Name[9] = {0}, Extension[4] = {0};

        while (segment)
        {
                lastSegment = segment;
                FatConvert83(segment, Name, Extension);
                Loc = FatLocateInDir((BYTE *)Name, (BYTE *)Extension, &bt, Self, pCurrentDir);
                if (!Loc.Found)
                {
                        return 0;
                }

                CurrentDir = Loc.Dir;
                pCurrentDir = &CurrentDir;
                segment = strtok(NULL, "/");
        }

        if (!lastSegment)
                return 0;

        FatConvert83(lastSegment, Name, Extension);
        return CurrentDir.Size;
}

void IDEDirectoryListing(DRIVE *Self, const char *Path)
{
        return;
}

void *SMRead(size_t LBA)
{
        if (CurrentDrive && CurrentDrive->ReadData && CurrentDrive->Valid && CurrentDrive->Present)
        {
                CurrentDrive->LBA = LBA;
                CurrentDrive->ReadData(CurrentDrive);
                return CurrentDrive->BufferA;
        }
        else
        {
                printf("ERROR: Disk Read Error (vague ik)\n");
                return NULL;
        }
}

void *SMReadFrom(size_t LBA, DRIVE *Drive)
{
        if (Drive && Drive->ReadData && Drive->Valid && Drive->Present)
        {
                Drive->LBA = LBA;
                Drive->ReadData(Drive);
                return Drive->BufferA;
        }
        else
        {
                printf("ERROR: Disk Read Error (vague ik)\n");
                return NULL;
        }
}

void *SMWrite(size_t LBA, void *Buf, size_t BufSize)
{
        if (CurrentDrive && CurrentDrive->WriteData && CurrentDrive->Valid && CurrentDrive->Present && BufSize <= sizeof(CurrentDrive->BufferA))
        {
                CurrentDrive->LBA = LBA;
                memcpy(CurrentDrive->BufferA, Buf, BufSize);
                CurrentDrive->WriteData(CurrentDrive);
                return CurrentDrive->BufferA;
        }
        else
        {
                printf("ERROR: Disk Write Error (vague ik)\n");
                return NULL;
        }
}

void *SMWriteTo(size_t LBA, void *Buf, size_t BufSize, DRIVE *Drive)
{
        if (Drive && Drive->WriteData && Drive->Valid && Drive->Present && BufSize <= sizeof(Drive->BufferA))
        {
                Drive->LBA = LBA;
                memcpy(Drive->BufferA, Buf, BufSize);
                Drive->WriteData(Drive);
                return Drive->BufferA;
        }
        else
        {
                printf("ERROR: Disk Write Error (vague ik)\n");
                return NULL;
        }
}

void SMChange(size_t New)
{
        if (New >= DriveCount)
        {
                printf("ERROR: Drive index out of range\n");
                return;
        }

        if (!Drives[New].Valid || !Drives[New].Present)
        {
                printf("ERROR: Drive is invalid or not present\n");
                return;
        }

        CurrentDrive = &Drives[New];
}

void SMInit(void)
{
        const size_t DevCount = pciGetDeviceCount();
        pci_device_t Dev[DevCount];
        pciGetDevices(Dev, 0, DevCount);

        DriveCount = 0;

        for (size_t i = 0; i < DevCount; ++i)
        {
                if (Dev[i].class_id == 0x01 && Dev[i].subclass_id == 0x01)
                {
                        IDEState.Dev = &Dev[i];
                        IDEFind(i);

                        for (int d = 0; d < 4; d++)
                        {
                                if (IDEState.IDEDev[d].Reserved != 1)
                                        continue;

                                if (DriveCount >= 64)
                                        break;

                                DRIVE *disk = &Drives[DriveCount];
                                disk->LBA = 0;
                                disk->ReadData = IDEReadData;
                                disk->WriteData = IDEWriteData;
                                disk->Present = true;
                                disk->Valid = true;
                                disk->Size = IDEState.IDEDev[d].Size;
                                disk->Channel = IDEState.IDEDev[d].Channel;
                                disk->DriveNum = IDEState.IDEDev[d].Drive;
                                memcpy(disk->Model, IDEState.IDEDev[d].Model, 41);
                                disk->Type = IDEState.IDEDev[d].Type;
                                disk->PCIDevIndex = i;
                                disk->ReadFile = IDEReadFile;
                                disk->FileSize = IDEFileSize;
                                disk->DirectoryListing = IDEDirectoryListing;
                                printf("dev%d: model=%s type=%d size=%dMB\n", DriveCount, disk->Model, disk->Type, (disk->Size * 512) / 1024 / 1024);
                                DriveCount++;
                        }
                }
        }

        for (size_t i = 0; i < DevCount; ++i)
        {
                if (Dev[i].class_id == 0x01 && Dev[i].subclass_id == 0x01)
                        continue;

                // Example: SATA / NVMe enumeration here
                // Add drives to Drives[] using DriveCount
        }

        if (DriveCount == 0)
                printf("WARNING: No drives found!\n");
        else if (DriveCount > 0)
                CurrentDrive = &Drives[0];
}

DRIVE *SMGetDrive(void)
{
        return CurrentDrive;
}

DRIVE *SMPopDrive(void)
{
        if (DriveStackSP > 0)
        {
                return DriveStack[--DriveStackSP];
        }

        return NULL;
}

DRIVE *SMPushDrive(DRIVE *Drive)
{
        if (DriveStackSP < 63)
        {
                DriveStack[DriveStackSP++] = Drive;
                return Drive;
        }

        return NULL;
}
