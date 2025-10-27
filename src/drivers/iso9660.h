#ifndef ISO9660_H
#define ISO9660_H

#include <stdint.h>
#include <drivers/disk.h>
#include <memory/alloc.h>
#include <memory/string.h>
#include <drivers/math.h>
#include <utils.h>
#include <tty/output/output.h>

#define ISO9660_IDEN CD001
#define ISO9660_FIRST_VOLUME_DESC 0x10

enum
{
        ISO9660_BOOT_RECORD,
        ISO9660_PRIMARY_DESC,
        ISO9660_SUPPLEMENTARY_DESC,
        ISO9660_VOLUME_PART_DESC,
        ISO9660_VOLUME_DESC_SET_TERMINATOR = 255,
};

typedef struct __attribute__((__packed__))
{
        char year[4];     /* 1..9999 */
        char month[2];    /* 1..12 */
        char day[2];      /* 1..31 */
        char hour[2];     /* 0..23 */
        char minute[2];   /* 0..59 */
        char second[2];   /* 0..59 */
        char second_h[2]; /* 0..99; we ignore this cus lazy */
        char gmt[2];      /* we ignore this cus lazy */
} iso9660DateTime_t;

typedef struct __attribute__((__packed__))
{
        char type;
        char iden[5]; /* CD001 */
        char version; /* 0x1 */
        char data[2041];
} iso9660VolumeDesc_t;

typedef struct __attribute__((__packed__))
{
        char type;           /* 0 */
        char iden[5];        /* CD001 */
        char version;        /* 0x1 */
        char bootsyside[32]; /* strA */
        char bootide[32];    /* strA */
        char data[1977];
} iso9660BootRecord_t;

typedef struct __attribute__((__packed__))
{
        char length_of_dir;
        char attr_record_len;
        uint32_t extend_lba[2];
        uint32_t data_length[2];
        struct __attribute__((__packed__))
        {
                uint8_t years_since_1900;
                uint8_t month;
                uint8_t day;
                uint8_t hour;
                uint8_t minute;
                uint8_t second;
                char gmt_offset;
        } date_and_time;
        union
        {
                struct __attribute__((__packed__))
                {
                        uint8_t hidden : 1;
                        uint8_t is_dir : 1;
                        uint8_t associated : 1;
                        uint8_t use_extended_attr : 1;
                        uint8_t owner_perms : 1;
                        uint8_t res1 : 1;
                        uint8_t res2 : 1;
                        uint8_t not_done : 1;
                } values;
                uint8_t raw;
        } flags;
        uint8_t unused_i_think_i_cant_remember[2];
        int16_t volume_seq[2];
        uint8_t length_of_dir_ident;
} iso9660Dir_t;

typedef struct __attribute__((__packed__))
{
        char type;    /* 1 */
        char iden[5]; /* CD001 */
        char version; /* 0x1 */
        char unused0;
        char syside[32]; /* strA */
        char volide[32]; /* strD */
        char unused1[8];
        uint32_t volume_space_size[2];
        uint8_t unused2[32];
        uint16_t volume_set_size[2];
        uint16_t volume_sequence_number[2];
        uint16_t logical_block_size[2];
        uint32_t path_table_size[2];
        uint32_t type_l_path_table;
        uint32_t opt_type_l_path_table;
        uint32_t type_m_path_table;
        uint32_t opt_type_m_path_table;
        iso9660Dir_t root_directory_record;
        char always0ithink;
        char volume_set_id[128];           /* strD */
        char publisher_id[128];            /* strA */
        char data_preparer_id[128];        /* strA */
        char application_id[128];          /* strA */
        char copyright_file_id[37];        /* strD */
        char abstract_file_id[37];         /* strD */
        char bibliographic_file_id[37];    /* strD */
        char volume_creation_date[17];     /* time */
        char volume_modification_date[17]; /* time */
        char volume_expiration_date[17];   /* time */
        char volume_effective_date[17];    /* time */
        char file_structure_version;
        char unused3;
        char application_use[512];
        char unused4[653];
} iso9660PrimaryDesc_t;

typedef struct __attribute__((__packed__))
{
        char type;    /* 255 */
        char ide[5];  /* CD001 */
        char version; /* 0x1 */
} iso9660VolumeDescSetTerminator_t;

typedef struct __attribute__((packed))
{
        uint8_t name_len;
        uint8_t extended_attr_len;
        uint32_t lba;
        uint16_t parent;
} iso9660PathTableRecord_t;

bool iso9660Initialized(void);
bool iso9660Init(void);
bool iso9660ReadVolumeDescriptors(void);
bool iso9660ParsePrimaryDescriptor(const iso9660VolumeDesc_t *desc);
uint32_t iso9660GetRootLba(void);
uint32_t iso9660GetLogicalBlockSize(void);
bool iso9660ReadDirectory(uint32_t lba, uint16_t *buffer);
void *iso9660ReadFile(iso9660Dir_t *file_entry);
bool iso9660ListDirectory(uint32_t directory_lba);
bool iso9660ListRoot(void);
bool iso9660ListDir(const char *path);
bool iso9660TraversePathTable(const char *path);
bool iso9660FindFile(const char *path, iso9660Dir_t *file_entry);
bool iso9660FindDirectory(const char *path, uint32_t *directory_lba);
bool iso9660FindInDirectory(uint32_t directory_lba, const char *filename, iso9660Dir_t *file_entry);

#endif
