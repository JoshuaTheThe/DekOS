#!/bin/bash

cd shell
./make.sh
cd ..

cd kernel

declare -a all_object_files
declare -a init_files heap_files isr_files pci_files drivers_files tty_files prog_files other_files

# Function to generate symbol file for an object file
generate_symbols() {
    local obj_file="$1"
    local sym_file="${obj_file%.o}.sym"
    
    # Use nm to extract symbols and filter for text segment functions
    nm -n "$obj_file" | grep " T " > "$sym_file"
    echo "Generated symbols: $sym_file"
}

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

            # Add debug symbols for better function information
            clang -m32 -march=i386 -I./src -c -ffreestanding -msoft-float -fno-builtin "$file" -o "$output_file" -Wall -Wextra
            #clang --analyze -Xanalyzer -analyzer-checker=core,deadcode,security,cplusplus,unix \
      #-m32 -march=i386 -I./src -c -ffreestanding -msoft-float -fno-builtin "$file"

            # Generate symbols for this object file
            generate_symbols "$output_file"

            # Categorize the object file
            categorize_object_file "$output_file"
        elif [[ -f "$file" && "$file" == *.lisp ]]; then
            # using the lisp compiler at https://github.com/JoshuaTheThe/LispC
            local base="${file#./src/}"
            base="${base%.s}"
            local output_file="obj/$base.o"
            all_object_files+=("$output_file")
            echo "Compiling $file -> $output_file"
            mkdir -p "$(dirname "$output_file")"
            lispc $file > "$output_file".s
            nasm "$output_file".s -o $output_file -felf32
            generate_symbols "$output_file"
            categorize_object_file "$output_file"
        elif [[ -f "$file" && "$file" == *.s ]]; then
            local base="${file#./src/}"
            base="${base%.s}"
            local output_file="obj/$base.o"
            all_object_files+=("$output_file")

            echo "Assembling $file -> $output_file"
            mkdir -p "$(dirname "$output_file")"
            clang -m32 -c -g "$file" -o "$output_file" -march=i386

            # Generate symbols for this object file
            generate_symbols "$output_file"

            # Categorize the object file
            categorize_object_file "$output_file"

        elif [ -d "$file" ]; then
            list_files_recursive "$file"
        fi
    done
}

# Function to create combined symbol table
create_symbol_table() {
    echo "Creating combined symbol table..."
    
    # Combine all symbol files into one
    nm -n bin/kernel.elf > obj/all_symbols.sym
    
    # Generate C header with function addresses
    echo "// Auto-generated function symbol table" > src/symbols.h
    echo "#ifndef SYMBOLS_H" >> src/symbols.h
    echo "#define SYMBOLS_H" >> src/symbols.h
    echo "#include <stdint.h>" >> src/symbols.h
    echo "" >> src/symbols.h
    
    # Parse symbols and create C array
    echo "typedef struct {" >> src/symbols.h
    echo "    const char* name;" >> src/symbols.h
    echo "    uint32_t *address;" >> src/symbols.h
    echo "} symbol_t;" >> src/symbols.h
    echo "" >> src/symbols.h
    echo "extern symbol_t kernel_symbols[];" >> src/symbols.h
    echo "extern const int kernel_symbols_count;" >> src/symbols.h
    echo "" >> src/symbols.h
    
    # Count symbols and create array
    local symbol_count=$(grep " T " obj/all_symbols.sym | wc -l)
    echo "const int kernel_symbols_count = $symbol_count;" > src/symbols.c
    echo "#include \"symbols.h\"" >> src/symbols.c
    echo "" >> src/symbols.c
    echo "symbol_t kernel_symbols[] = {" >> src/symbols.c
    
    # Add each symbol to the array
    grep " T " obj/all_symbols.sym | while read line; do
        local addr=$(echo "$line" | awk '{print $1}')
        local name=$(echo "$line" | awk '{print $3}')
        echo "    {\"$name\", (uint32_t*)0x$addr}," >> src/symbols.c
    done
    
    echo "};" >> src/symbols.c
    echo "" >> src/symbols.h
    echo "#endif // SYMBOLS_H" >> src/symbols.h
    
    echo "Generated symbols.h and symbols.c with $symbol_count functions"
}

# Create obj directory
mkdir -p obj

# Compile everything
echo "Compiling source files..."
list_files_recursive "./src"

# Create the symbol table

# Recompile symbols.c to include it in the build
#echo "Compiling symbols.c..."
#clang -m32 -march=i386 -I./src -c -ffreestanding -msoft-float -fno-builtin -g src/symbols.c -o obj/symbols.o
#all_object_files+=("obj/symbols.o")
#categorize_object_file "obj/symbols.o"

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
    
    # Generate final kernel symbol map
    echo "Generating final kernel symbol map..."
    nm -n bin/kernel.elf > bin/kernel.sym
    echo "Kernel symbol map: bin/kernel.sym"
else
    echo "Linking failed!"
fi

create_symbol_table

cd ..
