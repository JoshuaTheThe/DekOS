#!/bin/bash

nasm examples/example.s -o isodir/boot/example.ex -f bin

declare -a all_object_files
declare -a init_files heap_files isr_files pci_files drivers_files tty_files prog_files other_files

# Function to categorize object files based on directory structure
categorize_object_file() {
    local file="$1"
    
    case "$file" in
        obj/init/*|obj/boot/*|obj/gdt/*|obj/idt/*)
            init_files+=("$file")
            ;;
        obj/heap/*)
            heap_files+=("$file")
            ;;
        obj/isr/*)
            isr_files+=("$file")
            ;;
        obj/pci/*)
            pci_files+=("$file")
            ;;
        obj/drivers/*)
            drivers_files+=("$file")
            ;;
        obj/tty/*)
            tty_files+=("$file")
            ;;
        obj/programs/*)
            prog_files+=("$file")
            ;;
        *)
            other_files+=("$file")
            ;;
    esac
}

# Recursive compilation function
list_files_recursive() {
    local dir="$1"
    for file in "$dir"/*; do
        if [[ -f "$file" && "$file" == *.c ]]; then
            local base="${file#./src/}"
            base="${base%.c}"
            local output_file="obj/$base.o"
            all_object_files+=("$output_file")
            
            echo "Compiling $file -> $output_file"
            mkdir -p "$(dirname "$output_file")"

            # hehehehehehehe just a few
            clang -m32 -march=i386 -I./src -c -ffreestanding -fno-builtin "$file" -o "$output_file"
            
            # Categorize the object file
            categorize_object_file "$output_file"
        elif [[ -f "$file" && "$file" == *.s ]]; then
            local base="${file#./src/}"
            base="${base%.s}"
            local output_file="obj/$base.o"
            all_object_files+=("$output_file")
            
            echo "Assembling $file -> $output_file"
            mkdir -p "$(dirname "$output_file")"
            clang -m32 -c "$file" -o "$output_file" -march=i386 
            
            # Categorize the object file
            categorize_object_file "$output_file"
            
        elif [ -d "$file" ]; then
            list_files_recursive "$file"
        fi
    done
}

# Create obj directory
mkdir -p obj

# Compile everything
echo "Compiling source files..."
list_files_recursive "./src"

# Print categorized files
echo ""
echo "=== Categorized Object Files ==="
echo "Init files: ${init_files[@]}"
echo "Heap files: ${heap_files[@]}"
echo "ISR files: ${isr_files[@]}"
echo "PCI files: ${pci_files[@]}"
echo "Driver files: ${drivers_files[@]}"
echo "TTY files: ${tty_files[@]}"
echo "Program files: ${prog_files[@]}"
echo "Other files: ${other_files[@]}"

# Build the link command
echo ""
echo "=== Linking ==="
ld -m elf_i386 -T "linker.ld" -o "bin/kernel.elf" -nostdlib \
    "${init_files[@]}" \
    "${heap_files[@]}" \
    "${isr_files[@]}" \
    "${pci_files[@]}" \
    "${drivers_files[@]}" \
    "${tty_files[@]}" \
    "${prog_files[@]}" \
    "${other_files[@]}"
if [ $? -eq 0 ]; then
    echo "Linking successful: bin/kernel.elf"
else
    echo "Linking failed!"
fi

if [ "$1" == '--run' ]; then
        if grub-file --is-x86-multiboot bin/kernel.elf; then 
                echo multiboot confirmed 
                mkdir -p isodir/boot/grub 
                cp bin/kernel.elf isodir/boot/kernel.bin 
                cp grub.cfg isodir/boot/grub/grub.cfg 
                cp examples/example.ex isodir/boot/example.ex
                grub-mkrescue -o dekos.iso isodir 
                qemu-system-i386 -cdrom dekos.iso -m 128 -monitor stdio -boot d
        else 
                echo the file is not multiboot 
        fi 
fi
