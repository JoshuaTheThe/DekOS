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

# C toolchain
override DEFAULT_KCC := clang
$(eval $(call DEFAULT_VAR,KCC,$(DEFAULT_KCC)))

override DEFAULT_KLD := ld
$(eval $(call DEFAULT_VAR,KLD,$(DEFAULT_KLD)))

override DEFAULT_KRUSTC := rustc +nightly
$(eval $(call DEFAULT_VAR,KRUSTC,$(DEFAULT_KRUSTC)))

override DEFAULT_KZIGC := zig
$(eval $(call DEFAULT_VAR,KZIGC,$(DEFAULT_KZIGC)))

override DEFAULT_KRUSTFLAGS :=
$(eval $(call DEFAULT_VAR,KRUSTFLAGS,$(DEFAULT_KRUSTFLAGS)))

# C flags
override DEFAULT_KCFLAGS := -pipe -Wall -Wextra -c -O0 -g
$(eval $(call DEFAULT_VAR,KCFLAGS,$(DEFAULT_KCFLAGS)))

override DEFAULT_KCPPFLAGS :=
$(eval $(call DEFAULT_VAR,KCPPFLAGS,$(DEFAULT_KCPPFLAGS)))

override DEFAULT_KNASMFLAGS := -F dwarf
$(eval $(call DEFAULT_VAR,KNASMFLAGS,$(DEFAULT_KNASMFLAGS)))

override DEFAULT_KLDFLAGS :=
$(eval $(call DEFAULT_VAR,KLDFLAGS,$(DEFAULT_KLDFLAGS)))

override DEFAULT_KZIGFLAGS :=
$(eval $(call DEFAULT_VAR,KZIGFLAGS,$(DEFAULT_KZIGFLAGS)))

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
    -fstack-protector-strong \
    -fstack-check \
    -mstack-protector-guard=global

override KZIGFLAGS += \
    build-obj \
    -target i386-freestanding \
    -mcpu i386 \
    -femit-implib \
    -fno-stack-check \
    $(DEFAULT_KZIGFLAGS)

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

# FIXED: Use built-in i686-unknown-linux-gnu target
override KRUSTFLAGS += \
    --target i686-unknown-linux-gnu \
    -C panic=abort \
    -C opt-level=0 \
    -C debuginfo=2 \
    -C lto=off \
    -C codegen-units=1 \
    -C relocation-model=static \
    -C link-arg=-nostartfiles \
    -C link-arg=-static \
    -C link-args=-e_start \
    $(DEFAULT_KRUSTFLAGS)

override KLDFLAGS += \
    -m elf_i386 \
    -nostdlib \
    -static \
    -z max-page-size=0x1000 \
    -T src/linker.ld

# Source files
override CFILES := $(shell cd src && find -L * -type f -name '*.c' | LC_ALL=C sort)
override ASFILES := $(shell cd src && find -L * -type f -name '*.s' | LC_ALL=C sort)
override NASMFILES := $(shell cd src && find -L * -type f -name '*.asm' | LC_ALL=C sort)
override RUSTFILES := init/init.rs#$(shell cd src && find -L * -type f -name '*.rs' | LC_ALL=C sort)
override ZIGFILES := $(shell cd src && find -L * -type f -name '*.zig' | LC_ALL=C sort)

# Object files
override COBJ := $(addprefix obj/,$(CFILES:.c=.c.o))
override ASOBJ := $(addprefix obj/,$(ASFILES:.s=.s.o))
override NASMOBJ := $(addprefix obj/,$(NASMFILES:.asm=.asm.o))
override RUSTOBJ := $(addprefix obj/,$(RUSTFILES:.rs=.rs.o))
override ZIGOBJ := $(addprefix obj/,$(ZIGFILES:.zig=.zig.o))

override OBJ := $(COBJ) $(ASOBJ) $(NASMOBJ) $(RUSTOBJ) $(ZIGOBJ)
override HEADER_DEPS := $(addprefix obj/,$(CFILES:.c=.c.d))

.PHONY: all
all: bin/$(OUTPUT)

bin/$(OUTPUT): $(OBJ)
	mkdir -p "$$(dirname $@)"
	$(KLD) $(OBJ) $(KLDFLAGS) -o $@

-include $(HEADER_DEPS)

# Zig compilation - with proper quoting
obj/%.zig.o: src/%.zig
	mkdir -p "$$(dirname $@)"
	cd "$$(dirname $@)" && $(KZIGC) build-obj "$(abspath $<)" -target x86-freestanding -mcpu i686 -fno-stack-check --name $(basename $(notdir $<)).zig

# C compilation
obj/%.c.o: src/%.c
	mkdir -p "$$(dirname $@)"
	$(KCC) $(KCFLAGS) $(KCPPFLAGS) -c $< -o $@

# Assembly compilation (GAS syntax)
obj/%.s.o: src/%.s
	mkdir -p "$$(dirname $@)"
	$(KCC) $(ASFLAGS) -c $< -o $@

# Assembly compilation (NASM syntax)
obj/%.asm.o: src/%.asm
	mkdir -p "$$(dirname $@)"
	nasm $(KNASMFLAGS) $< -o $@

# FIXED: Rust compilation with proper output
obj/%.rs.o: src/%.rs
	mkdir -p "$$(dirname $@)"
	$(KRUSTC) $(KRUSTFLAGS) --crate-type lib --emit obj=$@ $<

.PHONY: clean
clean:
	rm -rf obj
	rm -rf bin

.PHONY: disk
disk:
	mkdir -p bin
	dd if=/dev/zero of=bin/fat32.img bs=1M count=64
	mkfs.vfat -F 32 -n SLAVE_DISK bin/fat32.img

.PHONY: run
run:
	./util/run.sh

.PHONY: rust-deps
rust-deps:
	rustup target add i686-unknown-linux-gnu
	rustup component add rust-src
	rustup component add llvm-tools-preview

.PHONY: zig-clean
zig-clean:
	rm -rf obj/*.zig.o

.PHONY: rust-clean
rust-clean:
	rm -rf obj/*.rs.o
	