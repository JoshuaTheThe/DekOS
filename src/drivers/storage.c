#include <drivers/storage.h>
#include <memory/string.h>
#include <math.h>

DRIVE Drives[64]; /* 00 - 63 */
DRIVE *CurrentDrive = NULL;
size_t DriveCount;

void IDEReadData(DRIVE *Self)
{
        IDEReadSectors(Self->DriveNum, 1, Self->LBA, 0x10, (unsigned int)Self->BufferA);
}

void IDEWriteData(DRIVE *Self)
{
        IDEWriteSectors(Self->DriveNum, 1, Self->LBA, 0x10, (unsigned int)Self->BufferA);
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
