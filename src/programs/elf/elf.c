#include <programs/elf/elf.h>
#include <tty/output.h>
#include <memory/alloc.h>
#include <memory/string.h>

/**
 * elfCheckFile - check whether it is an elf file
 * @hdr the program's header, or the start of the program.
 * @returns whether the program header is valid
 */
bool elfCheckFile(elf32EHeader_t *hdr)
{
        /* NULL Check */
        if (!hdr)
                return false;

        const bool correct_magic = (hdr->identifier[EI_MAG0] == ELF_MAGIC_0) &&
                                   (hdr->identifier[EI_MAG1] == ELF_MAGIC_1) &&
                                   (hdr->identifier[EI_MAG2] == ELF_MAGIC_2) &&
                                   (hdr->identifier[EI_MAG3] == ELF_MAGIC_3);
        return correct_magic;
}       /* end of elfCheckFile */

/**
 * elfCheckSupported - check whether the elf file is of a supported type
 * @hdr the prorgam's header
 * @returns whether it is supported or not
 */
bool elfCheckSupported(elf32EHeader_t *hdr)
{
        /* NULL+Invalid Check */
        if (!elfCheckFile(hdr))
        {
                //printf("FILE IS NOT VALID\n");
                return false;
        }

        const bool correct_arch     = (hdr->identifier[EI_CLASS] == ELF_CLASS_32) &&
                                      (hdr->machine == EM_386);
        const bool correct_data_fmt = (hdr->identifier[EI_DATA] == ELF_DATA_2_LSB);
        const bool correct_version  = (hdr->version == EV_CURRENT);
        const bool correct_text_fmt = (hdr->type == ET_REL || hdr->type == ET_EXEC);
        const bool is_supported     = (correct_arch && correct_data_fmt &&
                                       correct_version && correct_text_fmt);
        //printf("IS SUPPORTED: %d\n", is_supported);
        return is_supported;
}       /* end of elfCheckSupported */

/**
 * elfLoadRel - load a relative elf file
 * @returns the header's entry
 */
void *elfLoadRel(elf32EHeader_t *hdr)
{
        int result;
        /* Cannot be shortened, it must be sequential */
        result = elfLoadStageOne(hdr);
        if (result == ELF_ERROR)
        {
                //printf("STAGE ONE FAILED\n");
                return (void*)-1;
        }
        result = elfLoadStageTwo(hdr);
        if (result == ELF_ERROR)
        {
                //printf("STAGE TWO FAILED\n");
                return (void*)-1;
        }
        //printf("HEADER ENTRY: %x\n", hdr->entry);
        return (void *)hdr->entry;
}       /* end of elfLoadRel */

/**
 * elfLoadFile - load an elf file
 * @returns the header's entry
 */
void *elfLoadFile(void *file)
{
        elf32EHeader_t *header = (elf32EHeader_t *)file;
        /* NULL+Invalid+Supported Check */
        if (!elfCheckSupported(header))
                return NULL;
        switch (header->type)
        {
                case ET_EXEC:
                default:
                        return NULL;
                case ET_REL:
                        return elfLoadRel(header);
        }
        /* is never reached */
}       /* end of elfLoadFile */

/**
 * elfSectionHeader - @returns the header for the sections
 */
elf32SectionHeader_t *elfSectionHeader(elf32EHeader_t *hdr)
{
        return (elf32SectionHeader_t *)((int)hdr + hdr->shoff);
}       /* end of elfSectionHeader */

/**
 * elfSection - @returns the header for a section
 */
elf32SectionHeader_t *elfSection(elf32EHeader_t *hdr, size_t idx)
{
        if (!hdr || idx >= hdr->shnum)
                return NULL;
        return &elfSectionHeader(hdr)[idx];
}       /* end of elfSection */

/**
 * elfStringTable - @returns the string table from the file's string section (?)
 */
char *elfStringTable(elf32EHeader_t *hdr)
{
        if (hdr->shstrndx == SHN_UNDEF)
                return NULL;
        return (char *)hdr + elfSection(hdr, hdr->shstrndx)->sh_offset;
}       /* end of elfStringTable */

/**
 * elfLookupString - @returns the string at an offset in the string section
 */
char *elfLookupString(elf32EHeader_t *hdr, int offset)
{
        char * strtab = elfStringTable(hdr);
        char * result = (strtab) ? (strtab + offset) : NULL;
        return result;
}       /* end of elfLookupString */

/**
 * elfLookupSymbol - @returns the address of an external symbol, (stub)
 */
void *elfLookupSymbol(const char * const name)
{
        (void)name;
        return NULL;
}       /* end of elfLookupSymbol */

/**
 * elfGetSymbolExternal - @returns the value of an external symbol
 */
int elfGetSymbolExternal(elf32EHeader_t *hdr, int table, uint32_t idx, const elf32Symbol_t * const symbol, const elf32SectionHeader_t * const header)
{
        (void)table;
        (void)idx;
        const elf32SectionHeader_t * const strtab = elfSection(hdr, header->sh_link);
        if (!strtab)
                return ELF_ERROR;

        const char * const name = (const char * const)hdr + strtab->sh_offset + symbol->st_name;
        void *target = elfLookupSymbol(name);

        if (target)
                return (int)target;
        if (target && ELF32_ST_BIND(symbol->st_info) & STB_WEAK)
        {
                return 0;
        }

        return ELF_ERROR;
}       /* end of elfGetSymbolExternal */

/**
 * elfGetSymbolValue - @returns the value of the symbol at the given index in the given table.
 */
int elfGetSymbolValue(elf32EHeader_t *hdr, int table, uint32_t idx)
{
        /* NULL Check */
        if (table == SHN_UNDEF || idx == SHN_UNDEF)
                return SHN_UNDEF;
        const elf32SectionHeader_t * const symbolTable = elfSection(hdr, table);
        /* Bounds Check */
        const uint32_t symbolTableEntries = symbolTable->sh_size / symbolTable->sh_entsize;
        if (idx >= symbolTableEntries)
                return ELF_ERROR;
        
        const int32_t symbolAddress = (int32_t)hdr + symbolTable->sh_offset;
        const elf32Symbol_t * const symbol = &((elf32Symbol_t *)symbolAddress)[idx];

        if (symbol->st_shndx == SHN_UNDEF)
        {
                return elfGetSymbolExternal(hdr, table, idx, symbol, symbolTable);
        }
        else if(symbol->st_shndx == SHN_ABS)
        {
                /* Absolute symbol */
                return symbol->st_value;
        }
        else
        {
                /* Internally defined symbol */
                const elf32SectionHeader_t * const target = elfSection(hdr, symbol->st_shndx);
                if (!target)
                        return ELF_ERROR;
                return (int)((uintptr_t)hdr + target->sh_offset + symbol->st_value);
        }
}       /* end of elfGetSymbolValue */

/**
 * elfLoadStageOne - apply the first stage for relocation
 */
int elfLoadStageOne(elf32EHeader_t *hdr)
{
        elf32SectionHeader_t * const sections = elfSectionHeader(hdr);
        uint32_t i;

        for (i = 0; i < hdr->shnum; ++i)
        {
                elf32SectionHeader_t * const section = &sections[i];
                if (section->sh_type == SHT_NOBITS && section->sh_size && section->sh_flags & SHF_ALLOC)
                {
                        void *mem = malloc(section->sh_size);
                        memset(mem, 0, section->sh_size);
                        section->sh_offset = (int)mem - (int)hdr;
                        //printf("DEBUG: Allocated memory for section (%x)\n", section->sh_size);
                }
        }

        return 0;
}       /* end of elfLoadStageOne */

/**
 * elfDoRelocation - does what it says on the tin, helper function for numerous functions, to avoid nesting
 */
int elfDoRelocation(elf32EHeader_t *hdr, elf32Rel_t *rel, elf32SectionHeader_t *reltab)
{
        elf32SectionHeader_t *target = elfSection(hdr, reltab->sh_info);

        int addr = (int)hdr + target->sh_offset;
        int *ref = (int *)(addr + rel->r_offset);
        int symbolValue = 0;
        if (ELF32_R_SYM(rel->r_info) != SHN_UNDEF) 
        {
                symbolValue = elfGetSymbolValue(hdr, reltab->sh_link, ELF32_R_SYM(rel->r_info));
                if(symbolValue == ELF_ERROR)
                {
                        //printf("SYMBOL DOES NOT EXIST\n");
                        return ELF_ERROR;
                }
        }
        switch(ELF32_R_TYPE(rel->r_info))
        {
                case R_386_NONE:
                        break;
                case R_386_32:
                        /* Symbol + Offset */
                        //printf("RELOCATING %x -> %x+%x (%x)\n", *ref, symbolValue, *ref, DO_386_32(symbolValue, *ref));
                        *ref = DO_386_32(symbolValue, *ref);
                        break;
                case R_386_PC32:
                        /* Symbol + Offset - Section Offset */
                        //printf("RELOCATING %x -> %x+%x-%x (%x)\n", *ref, symbolValue, *ref, (int)ref, DO_386_PC32(symbolValue, *ref, (int)ref));
                        *ref = DO_386_PC32(symbolValue, *ref, (int)ref);
                        break;
                default:
                        /* Relocation type not supported, display error and return */
                        //printf("UNKNOWN RELOCATION\n");
                        return ELF_ERROR;
        }
        return symbolValue;
}       /* end of elfDoRelocation */

/**
 * elfApplyRelativeRelocation - does what it says on the tin, helper function for elfLoadStageTwo, to avoid nesting
 */
int elfApplyRelativeRelocation(elf32EHeader_t *hdr, elf32SectionHeader_t *section)
{
        uint32_t idx;
        for(idx = 0; idx < section->sh_size / section->sh_entsize; idx++)
        {
                elf32Rel_t * const reltab = &((elf32Rel_t *)((int)hdr + section->sh_offset))[idx];
                int result = elfDoRelocation(hdr, reltab, section);
                if(result == ELF_ERROR)
                {
                        //printf("RELOCATOIN FAILED\n");
                        return ELF_ERROR;
                }
        }

        return 0;
}       /* end of elfApplyRelativeRelocation */

/**
 * elfLoadStageTwo - apply the relocations
 */
int elfLoadStageTwo(elf32EHeader_t *hdr)
{
        elf32SectionHeader_t * const sections = elfSectionHeader(hdr);
        uint32_t i;
        for (i = 0; i < hdr->shnum; ++i)
        {
                elf32SectionHeader_t * const section = &sections[i];
                if(section->sh_type == SHT_REL && (elfApplyRelativeRelocation(hdr, section) != 0))
                {
                        return ELF_ERROR;
                }
        }
        return 0;
}       /* end of elfLoadStageTwo */

/**
 * elfLoadProgram - completely load and link the elf buffer, then create the process
 */
schedPid_t elfLoadProgram(uint8_t *file, size_t file_size, bool *iself, USERID User, int argc, char **argv)
{
        *(iself)=false;
        if (!elfCheckSupported((elf32EHeader_t *)file))
                return (schedPid_t){.valid=0};
        *(iself)=true;
        uint8_t *program_mem = malloc(file_size);
        if (!program_mem)
        {
                //printf("FAILED TO ALLOCATE PROGRAM\n");
                return (schedPid_t){.valid=0};
        }

        memcpy(program_mem, file, file_size);
        void *entry_point = elfLoadFile(program_mem);
        if (entry_point == (void*)-1)
        {
                free(program_mem);
                //printf("FAILED TO GET ENTRY POINT\n");
                return (schedPid_t){.valid=0};
        }

        uint32_t stack_size = 0x4000;
        uint8_t *stack_mem = malloc(stack_size);
        if (!stack_mem)
        {
                free(program_mem);
                //printf("FAILED TO ALLOCATE STACK\n");
                return (schedPid_t){.valid=0};
        }

        //printf("ENTRY POINT: %x\n", entry_point);
        schedPid_t pid = schedCreateProcess(
                "elf_program",    // Process name
                argv,             // No arguments
                argc,                // No argument count
                program_mem,      // Program memory
                (uint32_t)entry_point, // Entry offset
                stack_mem,        // Stack memory
                stack_size,       // Stack size
                schedGetCurrentPid(), // Parent process
                User
        );

        if (!pid.valid)
        {
                printf("FAILED TO CREATE PROC\n");
                return (schedPid_t){.valid=0};
        }

        return pid;
}       /* end of elfLoadProgram */
