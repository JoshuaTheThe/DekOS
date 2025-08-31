/* simple custom executable format (EX) */

#include <ex.h>

const char Sign[8] = "EXENNEA";

void print_header_info(const Header h)
{
        k_print("Signature: %s\n", h.Sign);
        k_print("Version: %d.%d.%d\n", h.major, h.minor, h.patch);
        k_print("Relocation Count: %d\n", h.RelocationCount);
        k_print("Relocation Segment: %d\n", h.RelocationsStart);
        k_print("Text Start: %d\n", h.TextSegmentOrg);
        k_print("Text Size: %d\n", h.TextSegmentSize);
        k_print("FunTableOrg: %d\n", h.FunTableOrg);
        k_print("Function Count: %d\n", h.FunctionCount);
        k_print("Stack Size: %d\n", h.StackSize);
}

void find_functions(const Header h, const char *buffer, Function *func)
{
        const char *func_table = buffer + h.FunTableOrg;
        for (int i = 0; i < h.FunctionCount; ++i)
        {
                const Function *fun = (const Function *)(func_table + i * sizeof(Function));
                memcpy(func[i].name, fun->name, 16);
                func[i].offset = fun->offset;
        }
}

void find_relocations(const Header h, const char *buffer, Relocation *relocations)
{
        const char *reloc_table = buffer + h.RelocationsStart;
        for (int i = 0; i < h.RelocationCount; ++i)
        {
                const Relocation *rel = (const Relocation *)(reloc_table + i * sizeof(Relocation));
                relocations[i].offset = rel->offset;
        }
}

void find_raw_data(const Header h, const char *buffer, char *raw)
{
        const char *text_segment = buffer + h.TextSegmentOrg;
        memcpy(raw, text_segment, h.TextSegmentSize);
}

void apply_relocations(const Header h, const Relocation *relocations, char *raw)
{
        for (int i = 0; i < h.RelocationCount; ++i)
        {
                int off = (relocations[i].offset - h.TextSegmentOrg);
                *(DWORD *)(raw + off) += (DWORD)raw;
        }
}

int execute(char *name, char *buffer, size_t buffer_size)
{
        if (buffer_size < sizeof(Header))
        {
                k_print("Buffer too small for header\n");
                return -1;
        }

        Header h;
        memcpy(&h, buffer, sizeof(Header));
        print_header_info(h);

        if (h.Sign[0] == '\0')
        {
                k_print("Invalid executable format\n");
                return -1;
        }

        if (strncmp(h.Sign, Sign, sizeof(Sign)) != 0)
        {
                k_print("Invalid signature. Not an EXENNEA executable.\n");
                return -1;
        }

        Function functions[h.FunctionCount];
        Relocation relocations[h.RelocationCount];

        cli();
        void *ExeMem = malloc(h.TextSegmentSize);
        if (!ExeMem)
        {
                sti();
                return -1;
        }

        find_functions(h, buffer, functions);
        find_relocations(h, buffer, relocations);
        find_raw_data(h, buffer, (char *)ExeMem);
        apply_relocations(h, relocations, (char *)ExeMem);

        // Execute the first function (entry point)
        if (h.FunctionCount > 0)
        {
                start_process(name, NULL, 0, ExeMem, functions[0].offset, NULL, h.StackSize);
                sti();
                return 0;
        }
        free(ExeMem);
        sti();
        return -1;
}
