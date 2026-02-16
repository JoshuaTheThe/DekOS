CC      := clang
LD      := ld
BA      := ba

CFLAGS  := -m32 -march=i386 -I. -I./API \
           -ffreestanding -msoft-float -fno-builtin \
           -Wall -Wextra -fPIC

LDFLAGS := -m elf_i386 -nostdlib

LIBOBJS := API/start.o \
           API/stdio.o \
           API/ini.o \
           API/string.o \
           API/dek.o

# Get all source files
C_SRCS  := $(wildcard *.c)
BA_SRCS := $(wildcard *.ba)
SRCS    := $(C_SRCS) $(BA_SRCS)

# Convert to object files
C_OBJS  := $(C_SRCS:.c=.o)
BA_OBJS := $(BA_SRCS:.ba=.o)
OBJS    := $(C_OBJS) $(BA_OBJS)

# Convert to ELF files (all .c and .ba become .elf)
ELFS    := $(SRCS:.c=.elf)   # This handles .c files
ELFS    := $(ELFS:.ba=.elf)   # This handles .ba files (overwrites, but same result)

USER_DIR := ../fat32dir/users/root
SYSTEM_DIR := ../fat32dir/system

all: $(ELFS) copy

# Compile C to object
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Compile BA to object (via assembly)
%.o: %.ba
	$(BA) < $< > $*.S
	$(CC) $(CFLAGS) -c $*.S -o $@

# Link to ELF
%.elf: %.o
	$(LD) $(LDFLAGS) -o $@ $(LIBOBJS) $<

copy: 
	# Copy shell.elf to system directory as "shell"
	cp shell.elf $(SYSTEM_DIR)/shell -f
	# Copy all other ELFs to user directory without .elf extension
	for elf in $(filter-out shell.elf, $(ELFS)); do \
		target=$(USER_DIR)/$${elf%.elf}; \
		cp $$elf $$target -f; \
	done

clean:
	rm -f *.o *.elf *.S

.PHONY: all clean copy