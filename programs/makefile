CC      := clang
LD      := ld

CFLAGS  := -m32 -march=i386 -I. -I./API \
           -ffreestanding -msoft-float -fno-builtin \
           -Wall -Wextra

LDFLAGS := -m elf_i386 -nostdlib

LIBOBJS := API/start.o \
           API/stdio.o \
           API/ini.o \
           API/string.o \
           API/dek.o

SRCS    := $(wildcard *.c)
OBJS    := $(SRCS:.c=.o)
ELFS    := $(SRCS:.c=.elf)

OUTDIR  := ../fat32dir

all: $(ELFS) copy

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.elf: %.o
	$(LD) $(LDFLAGS) -o $@ $(LIBOBJS) $<

copy:
	cp $(ELFS) $(OUTDIR) -rf

clean:
	rm -f *.o *.elf

.PHONY: all clean copy
