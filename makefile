override MAKEFLAGS += -rR

override OUTPUT := kernel

define DEFAULT_VAR =
    ifeq ($(origin $1),default)
        override $(1) := $(2)
    endif
    ifeq ($(origin $1),undefined)
        override $(1) := $(2)
    endif
endef

override DEFAULT_KCC := clang
$(eval $(call DEFAULT_VAR,KCC,$(DEFAULT_KCC)))

override DEFAULT_KLD := ld
$(eval $(call DEFAULT_VAR,KLD,$(DEFAULT_KLD)))

override DEFAULT_KCFLAGS := -pipe -Wall -Wextra -c -O0 -fno-omit-frame-pointer
$(eval $(call DEFAULT_VAR,KCFLAGS,$(DEFAULT_KCFLAGS)))

override DEFAULT_KCPPFLAGS :=
$(eval $(call DEFAULT_VAR,KCPPFLAGS,$(DEFAULT_KCPPFLAGS)))

override DEFAULT_KNASMFLAGS := -F dwarf
$(eval $(call DEFAULT_VAR,KNASMFLAGS,$(DEFAULT_KNASMFLAGS)))

override DEFAULT_KLDFLAGS :=
$(eval $(call DEFAULT_VAR,KLDFLAGS,$(DEFAULT_KLDFLAGS)))

override KCFLAGS += \
    -c \
    -std=gnu11 \
    -ffreestanding \
    -fno-builtin \
    -m32 \
    -mno-mmx \
    -Werror \
    -mcmodel=kernel \
    -O0 \
    -fno-omit-frame-pointer \
    -fstack-protector-strong \
    -fstack-check \
    -mstack-protector-guard=global

override ASFLAGS += \
    -c \
    -m32

override KCPPFLAGS := \
    -I src \
    -I limine \
    $(KCPPFLAGS) \
    -MMD \
    -MP

override KNASMFLAGS += \
    -Wall \
    $(DEFAULT_KNASMFLAGS) \
    -f elf32

override KLDFLAGS += \
    -m elf_i386 \
    -nostdlib \
    -static \
    -z max-page-size=0x1000 \
    -T src/linker.ld

override CFILES := $(shell cd src && find -L * -type f -name '*.c' | LC_ALL=C sort)
override ASFILES := $(shell cd src && find -L * -type f -name '*.s' | LC_ALL=C sort)
override NASMFILES := $(shell cd src && find -L * -type f -name '*.asm' | LC_ALL=C sort)
override OBJ := $(addprefix obj/,$(CFILES:.c=.c.o) $(ASFILES:.s=.s.o) $(NASMFILES:.asm=.asm.o))
override HEADER_DEPS := $(addprefix obj/,$(CFILES:.c=.c.d) $(ASFILES:.s=.s.d))

.PHONY: all
all: bin/$(OUTPUT)

bin/$(OUTPUT): $(OBJ)
	mkdir -p "$$(dirname $@)"
	$(KLD) $(OBJ) $(KLDFLAGS) -o $@

-include $(HEADER_DEPS)

obj/%.c.o: src/%.c
	mkdir -p "$$(dirname $@)"
	$(KCC) $(KCFLAGS) $(KCPPFLAGS) -c $< -o $@
obj/%.s.o: src/%.s
	mkdir -p "$$(dirname $@)"
	$(KCC) $(ASFLAGS) -c $< -o $@
obj/%.asm.o: src/%.asm
	mkdir -p "$$(dirname $@)"
	nasm $(KNASMFLAGS) $< -o $@

.PHONY: clean
clean:
	rm -rf obj
	rm bin/kernel

.PHONY: disk
disk:
	dd if=/dev/zero of=bin/fat32.img bs=1M count=64
	mkfs.vfat -F 32 -n SLAVE_DISK bin/fat32.img

.PHONY: run
run:
	./util/run.sh