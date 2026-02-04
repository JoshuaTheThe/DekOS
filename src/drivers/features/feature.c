#include <drivers/features/feature.h>
#include <drivers/features/sse.h>
#include <tty/output.h>

static char fxsave_region[512] __attribute__((aligned(16)));

/**
 * CPUID leaf 0x01:
 * bits 0–31  -> EDX
 * bits 32–63 -> ECX
 */

static char *Names[] = {
    /* EDX (0–31) */
    "FPU (Onboard x87 Floating Point Unit)",
    "VME (Virtual 8086 Mode Extensions)",
    "DE (Debugging Extensions)",
    "PSE (Page Size Extensions, 4 MB pages)",
    "TSC (Time Stamp Counter and RDTSC instruction)",
    "MSR (Model-Specific Registers, RDMSR/WRMSR)",
    "PAE (Physical Address Extension)",
    "MCE (Machine Check Exception)",
    "CX8 (CMPXCHG8B instruction)",
    "APIC (Onboard Advanced Programmable Interrupt Controller)",
    "*** (reserved)",
    "SEP (SYSENTER and SYSEXIT instructions)",
    "MTRR (Memory Type Range Registers)",
    "PGE (Page Global Enable bit in CR4)",
    "MCA (Machine Check Architecture)",
    "CMOV (Conditional Move instructions)",
    "PAT (Page Attribute Table)",
    "PSE36 (36-bit Page Size Extension)",
    "PSN (Processor Serial Number)",
    "CLFSH (CLFLUSH cache line flush instruction)",
    "*** (reserved)",
    "DS (Debug Store, save executed branch trace)",
    "ACPI (Thermal Control MSRs for ACPI)",
    "MMX (MMX 64-bit SIMD instructions)",
    "FXSR (FXSAVE and FXRSTOR instructions)",
    "SSE (Streaming SIMD Extensions)",
    "SSE2 (Streaming SIMD Extensions 2)",
    "SS (CPU Cache Implements Self-Snoop)",
    "HTT (Hyper-Threading Technology)",
    "TM (Thermal Monitor)",
    "IA64 (IA-64 Architecture Emulation)",
    "PBE (Pending Break Enable wakeup capability)",

    /* ECX / RCX (32–63) */
    "SSE3 (Streaming SIMD Extensions 3 / Prescott New Instructions)",
    "PCLMULQDQ (Carry-less Multiplication instruction)",
    "DTES64 (64-bit Debug Store)",
    "MONITOR (MONITOR and MWAIT instructions)",
    "DS-CPL (CPL-qualified Debug Store)",
    "VMX (Virtual Machine Extensions / Intel VT-x)",
    "SMX (Safer Mode Extensions / Trusted Execution)",
    "EST (Enhanced Intel SpeedStep Technology)",
    "TM2 (Thermal Monitor 2)",
    "SSSE3 (Supplemental Streaming SIMD Extensions 3)",
    "CNXT-ID (L1 Context ID)",
    "SDBG (Silicon Debug Interface)",
    "FMA (Fused Multiply-Add, FMA3)",
    "CX16 (CMPXCHG16B instruction)",
    "XTPR (Task Priority Message Disable)",
    "PDCM (Performance Debug Capability MSR)",
    "*** (reserved)",
    "PCID (Process Context Identifiers)",
    "DCA (Direct Cache Access for DMA)",
    "SSE4.1 (Streaming SIMD Extensions 4.1)",
    "SSE4.2 (Streaming SIMD Extensions 4.2)",
    "X2APIC (Extended xAPIC Support)",
    "MOVBE (Move Data After Swapping Bytes)",
    "POPCNT (Population Count instruction)",
    "TSC-DEADLINE (TSC Deadline APIC Timer)",
    "AES-NI (AES Encryption Instructions)",
    "XSAVE (Extended Processor State Save/Restore)",
    "OSXSAVE (OS supports XSAVE/XRSTOR)",
    "AVX (Advanced Vector Extensions, 256-bit SIMD)",
    "F16C (Half-Precision Floating-Point Conversion)",
    "RDRND (On-chip Random Number Generator)",
    "HYPERVISOR (Running under a Hypervisor)",
};

bool Features[64] = {0};

char *FeatureName(size_t Index)
{
        if (Index >= (sizeof(Names) / sizeof((Names)[0])))
                return "UNKNOWN";

        return Names[Index];
}

void FeaturesInit(void)
{
        printf("INFO: Finding Features\n");
        for (size_t i = 0; i < 64; ++i)
        {
                size_t index = i & 31;
                Features[i] = (i > 31) ? FeatureIsPresentECX(index) : FeatureIsPresentEDX(index);
                if (Features[i])
                {
                        const char *const name = FeatureName(i);
                        printf("INFO: Feature %s\n", name);
                }
        }

        /* Does more checks, so it has it's own function */
        bool SSEPresent = SSEIsAvailable();
        if (SSEPresent) /* if SSE(2) && FPU */
        {
                printf("INFO: SSE Is available, attempting to enable\n");
                SSEEnable();
                asm volatile(" fxsave %0 " ::"m"(fxsave_region));
                printf("INFO: SSE Is enabled\n");
        }
        else
        {
                printf("PANIC: SSE Is required for this operating system\n");
                while(1);
        }

        if (Features[2])
        {
                printf("INFO: DE Is available\n");
                FeatureCR4Enable(3);
        }

        if (Features[5])
        {
                printf("INFO: MSR Is available\n");
        }
        else
        {
                printf("PANIC: MSR Is not available\n");
                while(1);
        }
}

bool IsFeaturePresent(size_t Idx)
{
        return Features[Idx % 64];
}
