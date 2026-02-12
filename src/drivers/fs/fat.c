#include <drivers/fs/fat.h>
#include <memory/alloc.h>

FATBootSector FatReadBootSector(DRIVE *Drive)
{
        FATBootSector Sector;
        memcpy(&Sector, SMReadFrom(0, Drive), sizeof(Sector));
        return Sector;
}

DWORD FatTotalSectors(FATBootSector *Bt)
{
        return (Bt->bpb.TotalSectorsInVolume) ? Bt->bpb.TotalSectorsInVolume : Bt->bpb.ExtendedSectorCount;
}

DWORD FatSize(FATBootSector *Bt)
{
        return (Bt->bpb.SectorsPerFAT) ? Bt->bpb.SectorsPerFAT : Bt->as.fat32.SectorsPerFAT;
}

/* If FAT32 then ignore // will be 0 */
DWORD FatRootDirSize(FATBootSector *Bt)
{
        return ((Bt->bpb.RootDirectoryEntries * sizeof(FATDirectory)) + (Bt->bpb.BytesPerSector - 1)) / Bt->bpb.BytesPerSector;
}

DWORD FatFirstData(FATBootSector *Bt)
{
        return (Bt->bpb.ReservedSectors + (Bt->bpb.FATCount * FatSize(Bt)) + FatRootDirSize(Bt));
}

DWORD FatFirstFat(FATBootSector *Bt)
{
        return (Bt->bpb.ReservedSectors);
}

DWORD FatDataCount(FATBootSector *Bt)
{
        return FatTotalSectors(Bt) - (Bt->bpb.ReservedSectors + (Bt->bpb.FATCount * FatSize(Bt)) + FatRootDirSize(Bt));
}

DWORD FatClusterCount(FATBootSector *Bt)
{
        return FatDataCount(Bt) / Bt->bpb.SectorsPerCluster;
}

FATType FatIdentify(FATBootSector *Bt)
{
        DWORD TotalClusters = FatClusterCount(Bt);
        if (TotalClusters < 4085 &&
            (Bt->as.fat16.Signature == FAT_EXT_BOOT_RECORD_16_SIGN_A ||
             Bt->as.fat16.Signature == FAT_EXT_BOOT_RECORD_16_SIGN_B))
                return FAT_12;
        else if (TotalClusters < 65525 &&
                 (Bt->as.fat16.Signature == FAT_EXT_BOOT_RECORD_16_SIGN_A ||
                  Bt->as.fat16.Signature == FAT_EXT_BOOT_RECORD_16_SIGN_B))
                return FAT_16;
        else if (Bt->as.fat32.Signature == FAT_EXT_BOOT_RECORD_16_SIGN_A ||
                 Bt->as.fat32.Signature == FAT_EXT_BOOT_RECORD_16_SIGN_B)
                return FAT_32;
        return FAT_UNKNOWN;
}

DWORD FatFirstRoot(FATBootSector *Bt)
{
        return FatFirstData(Bt) - FatRootDirSize(Bt);
}

DWORD FatRootCluster(FATBootSector *Bt)
{
        return Bt->as.fat32.RootCluster;
}

DWORD FatFirstSectorForCluster(FATBootSector *Bt, DWORD Cluster)
{
        return ((Cluster - 2) * Bt->bpb.SectorsPerCluster) + FatFirstData(Bt);
}

/**
 *  NOTE - Original X gets modified
 */
void FatConvertPaddedToNull(char *X, size_t L)
{
        for (size_t i = 0; i < L; ++i)
                X[i] = (X[i] == ' ') ? '\0' : X[i];
}

DWORD FatNextCluster32(FATBootSector *bt, DWORD cluster, DRIVE *Drive)
{
        DWORD fatOffset = cluster * 4;
        DWORD fatSector = FatFirstFat(bt) + (fatOffset / bt->bpb.BytesPerSector);
        DWORD entOffset = fatOffset % bt->bpb.BytesPerSector;

        BYTE *sector = SMReadFrom(fatSector, Drive);
        DWORD entry = *(DWORD *)&sector[entOffset];

        return entry & 0x0FFFFFFF;
}

WORD FatNextCluster16(FATBootSector *bt, WORD cluster, DRIVE *Drive)
{
        DWORD fatOffset = cluster * 2;
        DWORD fatSector = FatFirstFat(bt) + (fatOffset / bt->bpb.BytesPerSector);
        DWORD entOffset = fatOffset % bt->bpb.BytesPerSector;

        BYTE *sector = SMReadFrom(fatSector, Drive);
        return *(WORD *)&sector[entOffset];
}

WORD FatNextCluster12(FATBootSector *bt, WORD cluster, DRIVE *Drive)
{
        DWORD fatOffset = cluster + (cluster / 2);
        DWORD fatSector = FatFirstFat(bt) + (fatOffset / bt->bpb.BytesPerSector);
        DWORD entOffset = fatOffset % bt->bpb.BytesPerSector;

        BYTE *sector = SMReadFrom(fatSector, Drive);
        WORD entry = *(WORD *)&sector[entOffset];

        if (cluster & 1)
                entry >>= 4;
        else
                entry &= 0x0FFF;

        return entry;
}

DWORD FatNextCluster(FATBootSector *bt, DWORD cluster, DRIVE *Drive)
{
        switch (FatIdentify(bt))
        {
        case FAT_UNKNOWN:
                return -1;
        case FAT_12:
                return FatNextCluster12(bt, cluster, Drive);
        case FAT_16:
                return FatNextCluster16(bt, cluster, Drive);
        case FAT_32:
                return FatNextCluster32(bt, cluster, Drive);
        }
}

FATFileLocation FatLocateInDir(BYTE Name[8], BYTE Ext[3], FATBootSector *bt, DRIVE *Drive, FATDirectory *Parent)
{
        if (Parent && !(Parent->Flags & FAT_DIRECTORY))
        {
                return (FATFileLocation){0};
        }

        /* TODO - Fat16 // 12 does this slightly differently */
        DWORD cluster = Parent ? Parent->EntryFirstClusterHigh << 16 | Parent->EntryFirstClusterLow : FatRootCluster(bt);

        const size_t bytes = bt->bpb.BytesPerSector;
        const size_t entries = bytes / sizeof(FATDirectory);

        FATDirectory Directory[entries];
        FATFileLocation Result = {0};
        memset(Directory, 0, sizeof(Directory));

        while (cluster < 0x0FFFFFF8)
        {
                const size_t first_root = FatFirstSectorForCluster(bt, cluster); // e.g. FatRootCluster(bt)
                for (size_t clusterOffset = 0; clusterOffset < bt->bpb.SectorsPerCluster; ++clusterOffset)
                {
                        memcpy(&Directory, SMReadFrom(first_root + clusterOffset, Drive), sizeof(Directory));

                        /* IF Byte 0 is 0, that is the end */
                        for (size_t i = 0; Directory[i].Name[0] && i < entries; ++i)
                        {
                                if (Directory[i].Name[0] == FAT_UNUSED)
                                        continue;
                                if (Directory[i].Ext[2] == 0x0F)
                                {
                                        printf(" [WARN] FAT Subsystem: Long File Names are not supported, Ignoring\n");
                                        continue;
                                }

                                if (!ncsstrncmp((char*)Directory[i].Name, (char*)Name, 8) && !ncsstrncmp((char*)Directory[i].Ext, (char*)Ext, 3))
                                {
                                        Result.Cluster = cluster;
                                        Result.Found = 1;
                                        Result.Index = i;
                                        Result.Offset = clusterOffset;
                                        Result.Dir = Directory[i];
                                        return Result;
                                }
                        }
                }

                cluster = FatNextCluster(bt, cluster, Drive);
        }

        return Result;
}

void *FatRead(BYTE Name[8], BYTE Ext[3], FATBootSector *bt, DRIVE *Drive, FATDirectory *Parent)
{
        if (Parent && !(Parent->Flags & FAT_DIRECTORY))
        {
                return NULL;
        }

        const size_t bytes = bt->bpb.BytesPerSector;
        FATFileLocation Location = FatLocateInDir(Name, Ext, bt, Drive, Parent);

        BYTE *Data = malloc(Location.Dir.Size + 1), *p = Data;
        memset(Data, 0, Location.Dir.Size + 1);
        DWORD cluster = Location.Dir.EntryFirstClusterHigh << 16 | Location.Dir.EntryFirstClusterLow;
        DWORD remaining = Location.Dir.Size;
        BYTE Sector[bytes];
        memset(Sector, 0, bytes);

        while (cluster < 0x0FFFFFF8)
        {
                DWORD sector = FatFirstSectorForCluster(bt, cluster);
                for (size_t off = 0; off < bt->bpb.SectorsPerCluster; ++off)
                {
                        memcpy(Sector, SMReadFrom(sector + off, Drive), bt->bpb.BytesPerSector);
                        if (remaining > bt->bpb.BytesPerSector)
                                memcpy(p, Sector, bt->bpb.BytesPerSector);
                        else
                                memcpy(p, Sector, remaining);
                        remaining -= bt->bpb.BytesPerSector;
                        p += bt->bpb.BytesPerSector;
                }

                cluster = FatNextCluster(bt, cluster, Drive);
        }

        return Data;
}

void FatConvert83(const char *path, char *NameOut, char *ExtOut)
{
        memset(NameOut, ' ', 8);
        memset(ExtOut, ' ', 3);

        size_t n = 0;
        while (*path && *path != '.' && n < 8)
                NameOut[n++] = *path++;

        if (*path == '.')
                path++;

        size_t e = 0;
        while (*path && e < 3 && (*path != '/'))
                ExtOut[e++] = *path++;
}

void FatTest(DRIVE *Drive)
{
        FATBootSector bt = FatReadBootSector(Drive);
        printf("\n-- FAT-XX TEST FOR DRIVE @%p --\n", Drive);
        printf("        fat.type                %d\n", FatIdentify(&bt));
        printf("        fat.total-sect          %d\n", FatTotalSectors(&bt));
        printf("        fat.size                %d\n", FatSize(&bt));
        printf("        fat.root-size           %d\n", FatRootDirSize(&bt));
        printf("        fat.first-dat           %d\n", FatFirstData(&bt));
        printf("        fat.first-fat           %d\n", FatFirstFat(&bt));
        printf("        fat.data-cnt            %d\n", FatDataCount(&bt));
        printf("        fat.cluster-cnt         %d\n", FatClusterCount(&bt));
        printf("        fat.first-root          %d\n", FatFirstRoot(&bt));
        printf("        fat.root-clust          %d\n", FatRootCluster(&bt));
        printf("        fat.first-root-cluster  %d\n", FatFirstSectorForCluster(&bt, FatRootCluster(&bt)));
}
