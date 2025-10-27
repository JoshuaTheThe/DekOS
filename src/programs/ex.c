/* simple custom executable format (EX) */

#include <programs/ex.h>

const char Sign[8] = "EXENNEA";

void print_header_info(const Header h)
{
        printf("Signature: %s\n", h.Sign);
        printf("Version: %d.%d.%d\n", h.major, h.minor, h.patch);
        printf("Relocation Count: %d\n", h.RelocationCount);
        printf("Relocation Segment: %d\n", h.RelocationsStart);
        printf("Text Start: %d\n", h.TextSegmentOrg);
        printf("Text Size: %d\n", h.TextSegmentSize);
        printf("FunTableOrg: %d\n", h.FunTableOrg);
        printf("Function Count: %d\n", h.FunctionCount);
        printf("Stack Size: %d\n", h.StackSize);
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
        cli();
        if (buffer_size < sizeof(Header))
        {
                printf("Buffer too small for header\n");
                return -1;
        }

        Header h;
        memcpy(&h, buffer, sizeof(Header));

        if (h.Sign[0] == '\0')
        {
                printf("Invalid executable format\n");
                return -1;
        }

        if (strncmp(h.Sign, Sign, sizeof(Sign)) != 0)
        {
                printf("Invalid signature. Not an EXENNEA executable.\n");
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

        uint8_t *stack = malloc(h.StackSize * 4);

        // Execute the first function (entry point)
        if (h.FunctionCount > 0)
        {
                StartProcess(name, NULL, 0, ExeMem, functions[0].offset, stack, h.StackSize);
                sti();
                return 0;
        }
        free(ExeMem);
        sti();
        return -1;
}
