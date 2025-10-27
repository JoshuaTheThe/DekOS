#ifndef ISO9660_H
#define ISO9660_H

#include <stdint.h>
#include <drivers/disk.h>
#include <heap/alloc.h>
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
} iso9660_date_time;

typedef struct __attribute__((__packed__))
{
        char type;
        char iden[5]; /* CD001 */
        char version; /* 0x1 */
        char data[2041];
} iso9660_volume_desc;

typedef struct __attribute__((__packed__))
{
        char type;           /* 0 */
        char iden[5];        /* CD001 */
        char version;        /* 0x1 */
        char bootsyside[32]; /* strA */
        char bootide[32];    /* strA */
        char data[1977];
} iso9660_boot_record;


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
        /* variable sized name and then a padding byte if odd */
} iso9660_directory;

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
        iso9660_directory root_directory_record;
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
} iso9660_primary_desc;

typedef struct __attribute__((__packed__))
{
        char type;    /* 255 */
        char ide[5];  /* CD001 */
        char version; /* 0x1 */
} iso9660_volume_desc_set_terminator;

typedef struct
{
        uint8_t name_len;
        uint8_t extended_attr_len;
        uint32_t lba;
        uint16_t parent;
} __attribute__((packed)) iso9660_path_table_record;

extern bool is_a_character(char x);
extern bool is_d_character(char x);
extern bool iso9660_init(void);
extern bool iso9660_read_volume_descriptors(void);
extern bool iso9660_parse_primary_desc(const iso9660_volume_desc *desc);
extern uint32_t iso9660_get_root_directory_lba(void);
extern uint32_t iso9660_get_logical_block_size(void);
extern bool iso9660_read_directory(uint32_t lba, uint16_t *buffer);
extern void *iso9660_read_file(iso9660_directory *file_entry);
extern bool iso9660_list_directory(uint32_t directory_lba);
extern void traverse_path_table(void);
extern bool iso9660_find_in_directory(uint32_t directory_lba, const char *filename, iso9660_directory *file_entry);
extern bool iso9660_find_directory(const char *path, uint32_t *directory_lba);
extern bool iso9660_find_file(const char *path, iso9660_directory *file_entry);

extern bool iso9660_initialized;

#endif
