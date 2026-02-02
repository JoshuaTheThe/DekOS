#ifndef ELF_H
#define ELF_H

#include <utils.h>
#include <programs/scheduler.h>
#include <memory/alloc.h>

// #include <utils.h>
// #include <memory/string.h>
// #include <tty/output/output.h>

#define EM_386 (3)
#define EV_CURRENT (1)

#define ELF_MAGIC_0 0x7F
#define ELF_MAGIC_1 'E'
#define ELF_MAGIC_2 'L'
#define ELF_MAGIC_3 'F'

#define ELF_DATA_2_LSB (1)
#define ELF_CLASS_32 (1)

#define ELF_IDENTIFIER 16

#define SHN_UNDEF (0x00) // Undefined/Not Present
#define SHN_ABS 0xFFF1

#define ELF_ERROR (-1)

#define ELF32_ST_BIND(INFO) ((INFO) >> 4)
#define ELF32_ST_TYPE(INFO) ((INFO) & 0x0F)

#define ELF32_R_SYM(INFO) ((INFO) >> 8)
#define ELF32_R_TYPE(INFO) ((uint8_t)(INFO))

#define DO_386_32(S, A) ((S) + (A))
#define DO_386_PC32(S, A, P) ((S) + (A) - (P))

typedef struct
{
        uint8_t identifier[ELF_IDENTIFIER];
        uint16_t type;
        uint16_t machine;
        uint32_t version;
        uint32_t entry;
        uint32_t phoff;
        uint32_t shoff;
        uint32_t flags;
        uint16_t hsize;
        uint16_t phent_size;
        uint16_t phnum;
        uint16_t shent_size;
        uint16_t shnum;
        uint16_t shstrndx;
} elf32EHeader_t;

typedef enum
{
        EI_MAG0,
        EI_MAG1,
        EI_MAG2,
        EI_MAG3,
        EI_CLASS,
        EI_DATA,
        EI_OSABI,
        EI_ABIVERSION,
        EI_PAD,
} elf32Ident_t;

typedef enum
{
        ET_NONE,
        ET_REL,
        ET_EXEC,
} elf32Type_t;

typedef enum
{
        SHT_NULL = 0,
        SHT_PROGBITS = 1,
        SHT_SYMTAB = 2,
        SHT_STRTAB = 3,
        SHT_RELA = 4,
        SHT_NOBITS = 8,
        SHT_REL = 9,
} elf32SectionHeaderTable_t;

typedef enum
{
        SHF_WRITE = 0x01,
        SHF_ALLOC = 0x02
} elf32SectionHeaderTableAttr_t;

typedef struct
{
        uint32_t sh_name;
        uint32_t sh_type;
        uint32_t sh_flags;
        uint32_t sh_addr;
        uint32_t sh_offset;
        uint32_t sh_size;
        uint32_t sh_link;
        uint32_t sh_info;
        uint32_t sh_addralign;
        uint32_t sh_entsize;
} elf32SectionHeader_t;

typedef struct
{
        uint32_t st_name;
        uint32_t st_value;
        uint32_t st_size;
        uint8_t st_info;
        uint8_t st_other;
        uint16_t st_shndx;
} elf32Symbol_t;

typedef enum
{
        STB_LOCAL = 0,  // Local scope
        STB_GLOBAL = 1, // Global scope
        STB_WEAK = 2    // Weak, (ie. __attribute__((weak)))
} elf32SymbolTableBindings_t;

typedef enum
{
        STT_NOTYPE = 0, // No type
        STT_OBJECT = 1, // Variables, arrays, etc.
        STT_FUNC = 2    // Methods or functions
} elf32SymbolTableTypes_t;

typedef struct
{
        int32_t r_offset;
        uint32_t r_info;
} elf32Rel_t;

typedef struct
{
        int32_t r_offset;
        uint32_t r_info;
        int32_t r_addend;
} elf32RelA_t;

typedef enum
{
        R_386_NONE = 0, // No relocation
        R_386_32 = 1,   // Symbol + Offset
        R_386_PC32 = 2  // Symbol + Offset - Section Offset
} elf32RelocationTableTypes_t;

typedef struct
{
        uint32_t p_type;
        uint32_t p_offset;
        uint32_t p_vaddr;
        uint32_t p_paddr;
        uint32_t p_filesz;
        uint32_t p_memsz;
        uint32_t p_flags;
        uint32_t p_align;
} elf32ProgramHeader_t;

bool elfCheckFile(elf32EHeader_t *hdr);
bool elfCheckSupported(elf32EHeader_t *hdr);
void *elfLoadRel(elf32EHeader_t *hdr);
void *elfLoadFile(void *file);
elf32SectionHeader_t *elfSectionHeader(elf32EHeader_t *hdr);
elf32SectionHeader_t *elfSection(elf32EHeader_t *hdr, size_t idx);
char *elfStringTable(elf32EHeader_t *hdr);
char *elfLookupString(elf32EHeader_t *hdr, int offset);
int elfGetSymbolValue(elf32EHeader_t *hdr, int table, uint32_t idx);
int elfGetSymbolExternal(elf32EHeader_t *hdr, int table, uint32_t idx, const elf32Symbol_t *const symbol, const elf32SectionHeader_t *const header);
int elfLoadStageOne(elf32EHeader_t *hdr);
int elfLoadStageTwo(elf32EHeader_t *hdr);
schedPid_t elfLoadProgram(uint8_t *file, size_t file_size, bool *iself, USERID User, int argc, char **argv);

#endif
