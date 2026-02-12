CC      := clang
LD      := ld

CFLAGS  := -m32 -march=i386 -I. -I./API \
           -ffreestanding -msoft-float -fno-builtin \
           -Wall -Wextra -Os

LDFLAGS := -m elf_i386 -nostdlib

LIBOBJS := API/start.o \
           API/stdio.o \
           API/ini.o \
           API/string.o \
           API/dek.o

SRCS    := $(wildcard *.c)
OBJS    := $(SRCS:.c=.o)
ELFS    := $(SRCS:.c=.elf)

USER_DIR := ../fat32dir/users/root
SYSTEM_DIR := ../fat32dir/system

all: $(ELFS) copy

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.elf: %.o
	$(LD) $(LDFLAGS) -o $@ $(LIBOBJS) $<

copy: 
	# Copy shell.elf to root
	cp shell.elf $(SYSTEM_DIR) -f
	# Copy all other ELFs to user directory
	for elf in $(filter-out shell.elf, $(ELFS)); do \
		cp $$elf $(USER_DIR) -f; \
	done

clean:
	rm -f *.o *.elf

.PHONY: all clean copy