/* The bootloader will look at this image and start execution at the symbol
   designated as the entry point. */
ENTRY(_start)

/* Tell where the various sections of the object files will be put in the final
   kernel image. */
SECTIONS
{
	/* It used to be universally recommended to use 1M as a start offset,
	   as it was effectively guaranteed to be available under BIOS systems.
	   However, UEFI has made things more complicated, and experimental data
	   strongly suggests that 2M is a safer place to load. In 2016, a new
	   feature was introduced to the multiboot2 spec to inform bootloaders
	   that a kernel can be loaded anywhere within a range of addresses and
	   will be able to relocate itself to run from such a loader-selected
	   address, in order to give the loader freedom in selecting a span of
	   memory which is verified to be available by the firmware, in order to
	   work around this issue. This does not use that feature, so 2M was
	   chosen as a safer option than the traditional 1M. */
	. = 2M;

        .text : {
                *(.multiboot)
                *(.text)
                text_start = .;
                *(.text*)
                text_end = .;
        }
        
        .rodata : {
                rodata_start = .;
                *(.rodata*)
                rodata_end = .;
        }
        
        .data : {
                data_start = .;
                *(.data)
                data_end = .;
        }
        
        .bss : {
                bss_start = .;
                *(COMMON)
                *(.bss)
                bss_end = .;
        }

        . = . + 0x1000;
        _heap_start = .;
        . = . + 0x2000000; /* 32Mb heap */
        _heap_end = .;
        _heap_map_start = .;
        . = . + 0x500000;
        _heap_map_end = .;
        _allocations = .;
        . = . + 0x2000000;
        _allocations_end = .;

	/* The compiler may produce other sections, by default it will put them in
	   a segment with the same name. Simply add stuff here as needed. */
}