#include <programs/elf/elf.h>
#include <tty/output.h>
#include <memory/alloc.h>
#include <memory/string.h>

static elf_extern_symbol_t *extern_symbols = NULL;
static int extern_symbol_count = 0;
static int extern_symbol_capacity = 0;

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
} /* end of elfCheckFile */

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
                // printf("FILE IS NOT VALID\n");
                return false;
        }

        const bool correct_arch = (hdr->identifier[EI_CLASS] == ELF_CLASS_32) &&
                                  (hdr->machine == EM_386);
        const bool correct_data_fmt = (hdr->identifier[EI_DATA] == ELF_DATA_2_LSB);
        const bool correct_version = (hdr->version == EV_CURRENT);
        const bool correct_text_fmt = (hdr->type == ET_REL || hdr->type == ET_EXEC);
        const bool is_supported = (correct_arch && correct_data_fmt &&
                                   correct_version && correct_text_fmt);
        // printf("IS SUPPORTED: %d\n", is_supported);
        return is_supported;
} /* end of elfCheckSupported */

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
                // printf("STAGE ONE FAILED\n");
                return (void *)-1;
        }
        result = elfLoadStageTwo(hdr);
        if (result == ELF_ERROR)
        {
                // printf("STAGE TWO FAILED\n");
                return (void *)-1;
        }
        // printf("HEADER ENTRY: %x\n", hdr->entry);
        return (void *)hdr->entry;
} /* end of elfLoadRel */

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
} /* end of elfLoadFile */

/**
 * elfSectionHeader - @returns the header for the sections
 */
elf32SectionHeader_t *elfSectionHeader(elf32EHeader_t *hdr)
{
        return (elf32SectionHeader_t *)((int)hdr + hdr->shoff);
} /* end of elfSectionHeader */

/**
 * elfSection - @returns the header for a section
 */
elf32SectionHeader_t *elfSection(elf32EHeader_t *hdr, size_t idx)
{
        if (!hdr || idx >= hdr->shnum)
                return NULL;
        return &elfSectionHeader(hdr)[idx];
} /* end of elfSection */

/**
 * elfStringTable - @returns the string table from the file's string section (?)
 */
char *elfStringTable(elf32EHeader_t *hdr)
{
        if (hdr->shstrndx == SHN_UNDEF)
                return NULL;
        return (char *)hdr + elfSection(hdr, hdr->shstrndx)->sh_offset;
} /* end of elfStringTable */

/**
 * elfLookupString - @returns the string at an offset in the string section
 */
char *elfLookupString(elf32EHeader_t *hdr, int offset)
{
        char *strtab = elfStringTable(hdr);
        char *result = (strtab) ? (strtab + offset) : NULL;
        return result;
} /* end of elfLookupString */

/**
 * elfGetSymbolExternal - @returns the value of an external symbol
 */
int elfGetSymbolExternal(elf32EHeader_t *hdr, int table, uint32_t idx, const elf32Symbol_t *const symbol, const elf32SectionHeader_t *const header)
{
        (void)table;
        (void)idx;
        const elf32SectionHeader_t *const strtab = elfSection(hdr, header->sh_link);
        if (!strtab)
                return ELF_ERROR;

        const char *const name = (const char *const)hdr + strtab->sh_offset + symbol->st_name;
        void *target = elfLookupSymbol(name);

        if (target)
                return (int)target;
        if (target && ELF32_ST_BIND(symbol->st_info) & STB_WEAK)
        {
                return 0;
        }

        return ELF_ERROR;
} /* end of elfGetSymbolExternal */

/**
 * elfGetSymbolValue - @returns the value of the symbol at the given index in the given table.
 */
int elfGetSymbolValue(elf32EHeader_t *hdr, int table, uint32_t idx)
{
        /* NULL Check */
        if (table == SHN_UNDEF || idx == SHN_UNDEF)
                return SHN_UNDEF;
        const elf32SectionHeader_t *const symbolTable = elfSection(hdr, table);
        /* Bounds Check */
        const uint32_t symbolTableEntries = symbolTable->sh_size / symbolTable->sh_entsize;
        if (idx >= symbolTableEntries)
                return ELF_ERROR;

        const int32_t symbolAddress = (int32_t)hdr + symbolTable->sh_offset;
        const elf32Symbol_t *const symbol = &((elf32Symbol_t *)symbolAddress)[idx];

        if (symbol->st_shndx == SHN_UNDEF)
        {
                return elfGetSymbolExternal(hdr, table, idx, symbol, symbolTable);
        }
        else if (symbol->st_shndx == SHN_ABS)
        {
                /* Absolute symbol */
                return symbol->st_value;
        }
        else
        {
                /* Internally defined symbol */
                const elf32SectionHeader_t *const target = elfSection(hdr, symbol->st_shndx);
                if (!target)
                        return ELF_ERROR;
                return (int)((uintptr_t)hdr + target->sh_offset + symbol->st_value);
        }
} /* end of elfGetSymbolValue */

/**
 * elfLoadStageOne - apply the first stage for relocation
 */
int elfLoadStageOne(elf32EHeader_t *hdr)
{
        elf32SectionHeader_t *const sections = elfSectionHeader(hdr);
        uint32_t i;

        for (i = 0; i < hdr->shnum; ++i)
        {
                elf32SectionHeader_t *const section = &sections[i];
                if (section->sh_type == SHT_NOBITS && section->sh_size && section->sh_flags & SHF_ALLOC)
                {
                        void *mem = malloc(section->sh_size);
                        memset(mem, 0, section->sh_size);
                        section->sh_offset = (int)mem - (int)hdr;
                        // printf("DEBUG: Allocated memory for section (%x)\n", section->sh_size);
                }
        }

        return 0;
} /* end of elfLoadStageOne */

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
                if (symbolValue == ELF_ERROR)
                {
                        // printf("SYMBOL DOES NOT EXIST\n");
                        return ELF_ERROR;
                }
        }
        switch (ELF32_R_TYPE(rel->r_info))
        {
        case R_386_NONE:
                break;
        case R_386_32:
                /* Symbol + Offset */
                // printf("RELOCATING %x -> %x+%x (%x)\n", *ref, symbolValue, *ref, DO_386_32(symbolValue, *ref));
                *ref = DO_386_32(symbolValue, *ref);
                break;
        case R_386_PC32:
                /* Symbol + Offset - Section Offset */
                // printf("RELOCATING %x -> %x+%x-%x (%x)\n", *ref, symbolValue, *ref, (int)ref, DO_386_PC32(symbolValue, *ref, (int)ref));
                *ref = DO_386_PC32(symbolValue, *ref, (int)ref);
                break;
        default:
                /* Relocation type not supported, display error and return */
                // printf("UNKNOWN RELOCATION\n");
                return ELF_ERROR;
        }
        return symbolValue;
} /* end of elfDoRelocation */

/**
 * elfApplyRelativeRelocation - does what it says on the tin, helper function for elfLoadStageTwo, to avoid nesting
 */
int elfApplyRelativeRelocation(elf32EHeader_t *hdr, elf32SectionHeader_t *section)
{
        uint32_t idx;
        for (idx = 0; idx < section->sh_size / section->sh_entsize; idx++)
        {
                elf32Rel_t *const reltab = &((elf32Rel_t *)((int)hdr + section->sh_offset))[idx];
                int result = elfDoRelocation(hdr, reltab, section);
                if (result == ELF_ERROR)
                {
                        // printf("RELOCATOIN FAILED\n");
                        return ELF_ERROR;
                }
        }

        return 0;
} /* end of elfApplyRelativeRelocation */

/**
 * elfLoadStageTwo - apply the relocations
 */
int elfLoadStageTwo(elf32EHeader_t *hdr)
{
        elf32SectionHeader_t *const sections = elfSectionHeader(hdr);
        uint32_t i;
        for (i = 0; i < hdr->shnum; ++i)
        {
                elf32SectionHeader_t *const section = &sections[i];
                if (section->sh_type == SHT_REL && (elfApplyRelativeRelocation(hdr, section) != 0))
                {
                        return ELF_ERROR;
                }
        }
        return 0;
} /* end of elfLoadStageTwo */

/**
 * elfLoadProgram - completely load and link the elf buffer, then create the process
 */
schedPid_t elfLoadProgram(uint8_t *file, size_t file_size, bool *iself, USERID User, int argc, char **argv)
{
        *(iself) = false;
        if (!elfCheckSupported((elf32EHeader_t *)file))
                return (schedPid_t){.valid = 0};
        *(iself) = true;
        uint8_t *program_mem = malloc(file_size);
        if (!program_mem)
        {
                return (schedPid_t){.valid = 0};
        }

        memcpy(program_mem, file, file_size);
        void *entry_point = elfLoadFile(program_mem);
        if (entry_point == (void *)-1)
        {
                free(program_mem);
                return (schedPid_t){.valid = 0};
        }

        uint32_t stack_size = 0x10000;
        uint8_t *stack_mem = malloc(stack_size);
        if (!stack_mem)
        {
                free(program_mem);
                return (schedPid_t){.valid = 0};
        }

        schedPid_t pid = schedCreateProcess(
            argv[0],
            argv,
            argc,
            program_mem,
            (uint32_t)entry_point,
            stack_mem,
            stack_size,
            schedGetCurrentPid(),
            User);

        if (!pid.valid)
        {
                return (schedPid_t){.valid = 0};
        }

        return pid;
} /* end of elfLoadProgram */

/**
 * elfLoadNoRun - load an ELF file but don't create a process
 * @returns pointer to the loaded ELF in memory, or NULL on failure
 */
void *elfLoadNoRun(uint8_t *file, size_t file_size)
{
        if (!elfCheckSupported((elf32EHeader_t *)file))
                return NULL;

        uint8_t *program_mem = malloc(file_size);
        if (!program_mem)
                return NULL;

        memcpy(program_mem, file, file_size);

        void *entry_point = elfLoadFile(program_mem);
        if (entry_point == (void *)-1)
        {
                free(program_mem);
                return NULL;
        }

        return program_mem;
}

void elfDumpSymbols(void *module)
{
        if (!module)
                return;

        elf32EHeader_t *hdr = (elf32EHeader_t *)module;
        elf32SectionHeader_t *sections = elfSectionHeader(hdr);

        printf("ELF Dump for module at %p:\n", module);
        printf("  Type: %s\n", hdr->type == ET_REL ? "REL" : hdr->type == ET_EXEC ? "EXEC"
                                                                                  : "OTHER");
        printf("  Sections: %d\n", hdr->shnum);

        for (int i = 0; i < hdr->shnum; i++)
        {
                elf32SectionHeader_t *sh = &sections[i];
                char *sec_name = elfLookupString(hdr, sh->sh_name);

                if (sh->sh_type == SHT_SYMTAB || sh->sh_type == SHT_DYNSYM)
                {
                        printf("\n  Symbol Table [%d]: %s (type=%d)\n", i, sec_name ? sec_name : "?", sh->sh_type);

                        // Get string table for this symbol table
                        if (sh->sh_link >= hdr->shnum)
                                continue;
                        elf32SectionHeader_t *strtab = &sections[sh->sh_link];
                        char *strings = (char *)module + strtab->sh_offset;

                        // Get symbols
                        elf32Symbol_t *syms = (elf32Symbol_t *)((uintptr_t)module + sh->sh_offset);
                        int sym_count = sh->sh_size / sh->sh_entsize;

                        for (int j = 0; j < sym_count; j++)
                        {
                                elf32Symbol_t *sym = &syms[j];
                                const char *sym_name = strings + sym->st_name;

                                // Skip empty names
                                if (sym->st_name == 0)
                                        continue;

                                const char *type_str = "?";
                                switch (ELF32_ST_TYPE(sym->st_info))
                                {
                                case STT_NOTYPE:
                                        type_str = "NOTYPE";
                                        break;
                                case STT_OBJECT:
                                        type_str = "OBJECT";
                                        break;
                                case STT_FUNC:
                                        type_str = "FUNC";
                                        break;
                                }

                                const char *bind_str = (ELF32_ST_BIND(sym->st_info) == STB_GLOBAL) ? "GLOBAL" : (ELF32_ST_BIND(sym->st_info) == STB_LOCAL) ? "LOCAL"
                                                                                                                                                           : "WEAK";

                                printf("    [%d] %s: %s %s value=0x%x size=%d\n",
                                       j, sym_name, bind_str, type_str, sym->st_value, sym->st_size);

                                // Highlight if it's main
                                if (strcmp(sym_name, "main") == 0)
                                {
                                        printf("      >>> FOUND MAIN! <<<\n");
                                }
                        }
                }
        }
}

void *elfSymbol(void *module, const char *name)
{
        if (!module || !name)
                return NULL;

        elf32EHeader_t *hdr = (elf32EHeader_t *)module;
        elf32SectionHeader_t *sections = elfSectionHeader(hdr);

        uint32_t module_base = (uint32_t)module;
        uint32_t link_address = 0x08000000; /* TODO - find actual link_addr */
        // Try to find a better link address from program headers
        // For now, use the entry point as a hint
        if (hdr->entry < 0x1000000)
        {
                link_address = hdr->entry & 0xF0000000;
        }

        for (int i = 0; i < hdr->shnum; i++)
        {
                elf32SectionHeader_t *sh = &sections[i];
                if (sh->sh_type != SHT_SYMTAB && sh->sh_type != SHT_DYNSYM)
                        continue;
                if (sh->sh_link >= hdr->shnum)
                        continue;

                elf32SectionHeader_t *strtab = &sections[sh->sh_link];
                char *strings = (char *)module + strtab->sh_offset;
                elf32Symbol_t *syms = (elf32Symbol_t *)((uintptr_t)module + sh->sh_offset);
                int sym_count = sh->sh_size / sh->sh_entsize;

                for (int j = 0; j < sym_count; j++)
                {
                        elf32Symbol_t *sym = &syms[j];
                        if (sym->st_name == 0)
                                continue;

                        const char *sym_name = strings + sym->st_name;

                        if (strcmp(sym_name, name) == 0)
                        {
                                uint32_t symbol_value = sym->st_value;
                                uint32_t actual_addr;

                                if (sym->st_shndx == SHN_ABS)
                                {
                                        actual_addr = symbol_value;
                                }
                                else
                                {
                                        actual_addr = symbol_value - link_address + module_base;
                                }

                                return (void *)actual_addr;
                        }
                }
        }

        return NULL;
}

void elfRegisterExternSymbol(const char *name, void *addr)
{
        if (extern_symbols == NULL)
        {
                extern_symbol_capacity = 16;
                extern_symbols = malloc(extern_symbol_capacity * sizeof(elf_extern_symbol_t));
                if (!extern_symbols)
                        return;
        }

        if (extern_symbol_count >= extern_symbol_capacity)
        {
                int new_capacity = extern_symbol_capacity * 2;
                elf_extern_symbol_t *new_array = malloc(new_capacity * sizeof(elf_extern_symbol_t));
                if (!new_array)
                        return;
                memcpy(new_array, extern_symbols, extern_symbol_count * sizeof(elf_extern_symbol_t));
                free(extern_symbols);
                extern_symbols = new_array;
                extern_symbol_capacity = new_capacity;
        }
        extern_symbols[extern_symbol_count].name = name;
        extern_symbols[extern_symbol_count].addr = addr;
        extern_symbol_count++;
}

void *elfLookupSymbol(const char *const name)
{
        for (int i = 0; i < extern_symbol_count; i++)
        {
                if (strcmp(extern_symbols[i].name, name) == 0)
                {
                        return extern_symbols[i].addr;
                }
        }
        return NULL;
}
