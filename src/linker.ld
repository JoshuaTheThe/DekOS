ENTRY(_start)

SECTIONS
{
	. = 1M;
        _kernel_start = .;

        /* First, ensure multiboot header is at the very beginning */
        .text : {
                *(.multiboot)
                *(.multiboot2)
                *(.text)
                *(.text.*)      /* Catch all .text.* sections */
                text_start = .;
                *(.text*)
                text_end = .;
        }
        
        .rodata : {
                rodata_start = .;
                *(.rodata)
                *(.rodata.*)    /* Catch all .rodata.* sections */
                *(.rodata*)     /* Catch any other rodata variants */
                *(.eh_frame)    /* Exception handling frames */
                *(.eh_frame_hdr)
                *(.comment)     /* Compiler comments */
                *(.comment.*)
                rodata_end = .;
        }
        
        .data : {
                data_start = .;
                *(.data)
                *(.data.*)      /* Catch all .data.* sections */
                *(.got)         /* Global offset table */
                *(.got.*)       /* GOT entries */
                *(.got.plt)     /* GOT PLT entries */
                *(.plt)         /* Procedure linkage table */
                *(.plt.*)
                *(.iplt)        /* Inline PLT */
                *(.igot.*)
                *(.toc)         /* Table of contents (PPC) */
                *(.toc.*)
                data_end = .;
        }
        
        .bss : {
                bss_start = .;
                *(COMMON)
                *(.bss)
                *(.bss.*)       /* Catch all .bss.* sections */
                *(COMMON.*)
                bss_end = .;
        }

        /* Catch any stragglers that didn't fit in above sections */
        . = ALIGN(4);
        .init : {
                *(.init)
                *(.init.*)
        }
        
        .fini : {
                *(.fini)
                *(.fini.*)
        }
        
        .ctors : {
                *(.ctors)
                *(.ctors.*)
                *(.init_array)
                *(.init_array.*)
        }
        
        .dtors : {
                *(.dtors)
                *(.dtors.*)
                *(.fini_array)
                *(.fini_array.*)
        }
        
        /* Exception handling sections */
        .gcc_except_table : {
                *(.gcc_except_table)
                *(.gcc_except_table.*)
        }
        
        .jcr : {
                *(.jcr)
        }
        
        /* Debug sections - usually stripped, but catch them just in case */
        .debug : {
                *(.debug)
                *(.debug.*)
        }
        
        .stab : {
                *(.stab)
                *(.stab.*)
        }
        
        .stabstr : {
                *(.stabstr)
                *(.stabstr.*)
        }
        
        /* ARM specific */
        .ARM.exidx : {
                *(.ARM.exidx*)
                *(.gnu.linkonce.armexidx.*)
        }
        
        .ARM.extab : {
                *(.ARM.extab*)
                *(.gnu.linkonce.armextab.*)
        }

        /* Ensure we catch everything else that might have slipped through */
        . = ALIGN(4);
        _kernel_end = .;
        _kernel_size = _kernel_end - _kernel_start;
        
        /* Discard any remaining sections we don't need */
        /DISCARD/ : {
                *(.note.GNU-stack)
                *(.gnu_debuglink)
                *(.gnu.lto_*)
                *(.comment)
                *(.comment.*)
        }
}