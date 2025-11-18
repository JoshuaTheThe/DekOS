/* simple custom executable format (EX) */

#include <programs/ex.h>

static const char Sign[8] = "EXENNEA";

void exHeaderInfo(const exHeader_t h)
{
        printf("Signature: %s\n", h.Sign);
        printf("Version: %d.%d.%d\n", h.major, h.minor, h.patch);
        printf("Relocation Count: %d\n", h.RelocationCount);
        printf("Relocation Segment: %d\n", h.RelocationsStart);
        printf("Text Start: %d\n", h.TextSegmentOrg);
        printf("Text Size: %d\n", h.TextSegmentSize);
        printf("FunTableOrg: %d\n", h.FunTableOrg);
        printf("exFunction_t Count: %d\n", h.FunctionCount);
        printf("Stack Size: %d\n", h.StackSize);
}

void exFindFunctions(const exHeader_t h, const char *buffer, exFunction_t *func)
{
        const char *func_table = buffer + h.FunTableOrg;
        for (size_t i = 0; i < h.FunctionCount; ++i)
        {
                const exFunction_t *fun = (const exFunction_t *)(func_table + i * sizeof(exFunction_t));
                memcpy(func[i].name, fun->name, 16);
                func[i].offset = fun->offset;
        }
}

void exFindRelocations(const exHeader_t h, const char *buffer, exRelocation_t *relocations)
{
        const char *reloc_table = buffer + h.RelocationsStart;
        for (size_t i = 0; i < h.RelocationCount; ++i)
        {
                const exRelocation_t *rel = (const exRelocation_t *)(reloc_table + i * sizeof(exRelocation_t));
                relocations[i].offset = rel->offset;
        }
}

void exFindRawData(const exHeader_t h, const char *buffer, char *raw)
{
        const char *text_segment = buffer + h.TextSegmentOrg;
        memcpy(raw, text_segment, (int)h.TextSegmentSize);
}

void exApplyRelocations(const exHeader_t h, const exRelocation_t *relocations, char *raw)
{
        for (size_t i = 0; i < h.RelocationCount; ++i)
        {
                size_t off = (size_t)relocations[i].offset;
                *(DWORD *)(raw + off) += (DWORD)raw;
        }
}

int exExecute(char *name, char *buffer, size_t buffer_size, schedPid_t parent)
{
        if (buffer_size < sizeof(exHeader_t))
        {
                printf("Buffer too small for header\n");
                return -1;
        }

        exHeader_t h;
        memcpy(&h, buffer, sizeof(exHeader_t));

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

        exFunction_t functions[h.FunctionCount];
        exRelocation_t relocations[h.RelocationCount];

        void *ExeMem = malloc(h.TextSegmentSize);
        if (!ExeMem)
        {
                return -1;
        }

        exFindFunctions(h, buffer, functions);
        exFindRelocations(h, buffer, relocations);
        exFindRawData(h, buffer, (char *)ExeMem);
        exApplyRelocations(h, relocations, (char *)ExeMem);

        uint8_t *stack = malloc(h.StackSize * 4);

        // Execute the first function (entry point)
        if (h.FunctionCount > 0)
        {
                return schedCreateProcess(name, NULL, 0, ExeMem, (uint32_t)functions[0].offset, stack, (uint32_t)h.StackSize, parent).num;
        }
        free(ExeMem);
        return -1;
}
