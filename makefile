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

override DEFAULT_ARCH := x86
$(eval $(call DEFAULT_VAR,ARCH,$(DEFAULT_ARCH)))

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
override DEFAULT_KCFLAGS := -pipe -Wall -Wextra -c -O0 -g -fno-strict-aliasing
$(eval $(call DEFAULT_VAR,KCFLAGS,$(DEFAULT_KCFLAGS)))

override DEFAULT_KCPPFLAGS :=
$(eval $(call DEFAULT_VAR,KCPPFLAGS,$(DEFAULT_KCPPFLAGS)))

override DEFAULT_KNASMFLAGS := -F dwarf -felf32
$(eval $(call DEFAULT_VAR,KNASMFLAGS,$(DEFAULT_KNASMFLAGS)))

override DEFAULT_KLDFLAGS :=
$(eval $(call DEFAULT_VAR,KLDFLAGS,$(DEFAULT_KLDFLAGS)))

override DEFAULT_KZIGFLAGS :=
$(eval $(call DEFAULT_VAR,KZIGFLAGS,$(DEFAULT_KZIGFLAGS)))

override DEFAULT_KFORTRAN := gfortran
$(eval $(call DEFAULT_VAR,KFORTRAN,$(DEFAULT_KFORTRAN)))

override DEFAULT_KFORTRANFLAGS :=
$(eval $(call DEFAULT_VAR,KFORTRANFLAGS,$(DEFAULT_KFORTRANFLAGS)))

# Architecture-specific flags (to be overridden by arch.mk)
override ARCH_CFLAGS :=
override ARCH_ASFLAGS :=
override ARCH_KLDFLAGS :=
override ARCH_KRUSTFLAGS :=
override ARCH_KZIGFLAGS :=
override ARCH_KFORTRANFLAGS :=
override ARCH_KCPPFLAGS :=
override ARCH_KNASMFLAGS := -felf32 -F dwarf
override ARCH_LINKER_SCRIPT := src/arch/$(ARCH)/linker.ld
override ARCH_OUTPUT_SUFFIX :=

# Include architecture-specific configuration
include src/arch/$(ARCH)/arch.mk

# Apply architecture flags (with defaults for x86)
override KCFLAGS += \
    -c \
    -std=gnu11 \
    -ffreestanding \
    -fno-builtin \
    -Werror \
    -fstack-protector-strong \
    -fstack-check \
    $(ARCH_CFLAGS)

override KFORTRANFLAGS += \
    -c \
    -ffreestanding \
    -fno-range-check \
    -fno-leading-underscore \
    -Wno-argument-mismatch \
    -ffixed-line-length-none \
    -ffree-form \
    $(ARCH_KFORTRANFLAGS)

override KZIGFLAGS += \
     build-obj \
    -fno-stack-check \
    $(ARCH_KZIGFLAGS) \
    $(DEFAULT_KZIGFLAGS)

override ASFLAGS += \
    -c \
    $(ARCH_ASFLAGS)

override KCPPFLAGS := \
    -I src \
    -I src/arch/$(ARCH) \
    -I limine \
    $(ARCH_KCPPFLAGS) \
    $(KCPPFLAGS) \
    -MMD \
    -MP

override KNASMFLAGS += \
    -Wall \
    $(ARCH_KNASMFLAGS) \
    $(DEFAULT_KNASMFLAGS)

override KRUSTFLAGS += \
    -C panic=abort \
    -C opt-level=0 \
    -C debuginfo=0 \
    -C lto=off \
    -C codegen-units=1 \
    -C force-unwind-tables=n \
    --cfg feature=\"panic_immediate_abort\" \
    -C link-arg=-nostartfiles \
    -C link-arg=-static \
    $(ARCH_KRUSTFLAGS) \
    $(DEFAULT_KRUSTFLAGS)

override KLDFLAGS += \
    -nostdlib \
    -static \
    -z max-page-size=0x1000 \
    $(DEFAULT_KLDFLAGS)

ifeq ($(findstring -T,$(ARCH_KLDFLAGS)),)
    ifneq ($(wildcard $(ARCH_LINKER_SCRIPT)),)
        override KLDFLAGS += -T $(ARCH_LINKER_SCRIPT)
    else
        override KLDFLAGS += -T src/linker.ld
    endif
else
    override KLDFLAGS += $(ARCH_KLDFLAGS)
endif

# Update output name for architecture
override OUTPUT := $(OUTPUT)$(ARCH_OUTPUT_SUFFIX)

# Source files - common + architecture specific
override CFILES := $(shell cd src && find -L * -type f -name '*.c' | grep -v "^arch/" | LC_ALL=C sort)
override CFILES += $(shell cd src && find -L arch/$(ARCH) -type f -name '*.c' 2>/dev/null | LC_ALL=C sort)

override ASFILES := $(shell cd src && find -L * -type f -name '*.s' | grep -v "^arch/" | LC_ALL=C sort)
override ASFILES += $(shell cd src && find -L arch/$(ARCH) -type f -name '*.s' 2>/dev/null | LC_ALL=C sort)

override NASMFILES := $(shell cd src && find -L * -type f -name '*.asm' | grep -v "^arch/" | LC_ALL=C sort)
override NASMFILES += $(shell cd src && find -L arch/$(ARCH) -type f -name '*.asm' 2>/dev/null | LC_ALL=C sort)

override RUSTFILES := $(shell cd src && find -L * -type f -name '*.rs' | grep -v "^arch/" | LC_ALL=C sort)
override RUSTFILES += $(shell cd src && find -L arch/$(ARCH) -type f -name '*.rs' 2>/dev/null | LC_ALL=C sort)

override ZIGFILES := $(shell cd src && find -L * -type f -name '*.zig' | grep -v "^arch/" | LC_ALL=C sort)
override ZIGFILES += $(shell cd src && find -L arch/$(ARCH) -type f -name '*.zig' 2>/dev/null | LC_ALL=C sort)

override FORFILES := $(shell cd src && find -L * -type f -name '*.for' | grep -v "^arch/" | LC_ALL=C sort)
override FORFILES += $(shell cd src && find -L arch/$(ARCH) -type f -name '*.for' 2>/dev/null | LC_ALL=C sort)

# Object files
override COBJ := $(addprefix obj/,$(CFILES:.c=.c.o))
override ASOBJ := $(addprefix obj/,$(ASFILES:.s=.s.o))
override NASMOBJ := $(addprefix obj/,$(NASMFILES:.asm=.asm.o))
override RUSTOBJ := $(addprefix obj/,$(RUSTFILES:.rs=.rs.o))
override ZIGOBJ := $(addprefix obj/,$(ZIGFILES:.zig=.zig.o))
override FOROBJ := $(addprefix obj/,$(FORFILES:.for=.for.o))

# Merged Rust object (to fix duplicate symbols)
RUST_MERGED := obj/rust_merged.o

# All objects except Rust ones (for the merge step)
NON_RUST_OBJ := $(COBJ) $(ASOBJ) $(NASMOBJ) $(ZIGOBJ) $(FOROBJ)

override HEADER_DEPS := $(addprefix obj/,$(CFILES:.c=.c.d))
override HEADER_DEPS += $(addprefix obj/,$(CFILES:.C=.C.d))

.PHONY: all
all: bin/$(OUTPUT)

# Merge all Rust objects into one
$(RUST_MERGED): $(RUSTOBJ)
	@echo " [INFO] Merging Rust objects to fix duplicate symbols..."
	ld -r $(filter -m%, $(ARCH_KLDFLAGS)) --allow-multiple-definition -o $@ $(RUSTOBJ)

# Link with merged Rust object
bin/$(OUTPUT): $(NON_RUST_OBJ) $(RUST_MERGED)
	mkdir -p "$$(dirname $@)"
	$(KLD) $(NON_RUST_OBJ) $(RUST_MERGED) $(KLDFLAGS) -o $@
	@echo " [INFO] Built $(OUTPUT) for architecture $(ARCH)"

-include $(HEADER_DEPS)

obj/%.for.o: src/%.for
	mkdir -p "$$(dirname $@)"
	$(KFORTRAN) $(KFORTRANFLAGS) $< -o $@

obj/%.zig.o: src/%.zig
	mkdir -p "$$(dirname $@)"
	cd "$$(dirname $@)" && $(KZIGC) $(KZIGFLAGS) "$(abspath $<)" --name $(basename $(notdir $<)).zig

obj/%.c.o: src/%.c
	mkdir -p "$$(dirname $@)"
	$(KCC) $(KCFLAGS) $(KCPPFLAGS) -c $< -o $@

obj/%.s.o: src/%.s
	mkdir -p "$$(dirname $@)"
	$(KCC) $(ASFLAGS) -c $< -o $@

obj/%.asm.o: src/%.asm
	mkdir -p "$$(dirname $@)"
	nasm $(KNASMFLAGS) $< -o $@

obj/%.rs.o: src/%.rs
	mkdir -p "$$(dirname $@)"
	$(KRUSTC) $(KRUSTFLAGS) --crate-type staticlib --emit obj=$@ $<

.PHONY: arch-x86
arch-x86:
	$(MAKE) ARCH=x86 all

image: bin/$(OUTPUT)
	@echo " [INFO] Creating raw disk image for $(ARCH)..."
	@mkdir -p bin
	@dd if=/dev/zero of=bin/polyglotOS-$(ARCH).img bs=1M count=64 2>/dev/null
	@dd if=bin/$(OUTPUT) of=bin/polyglotOS-$(ARCH).img bs=512 seek=1 conv=notrunc 2>/dev/null

.PHONY: clean
clean:
	rm -rf obj bin

.PHONY: disk
disk:
	mkdir -p bin
	dd if=/dev/zero of=bin/fat32.img bs=1M count=64
	mkfs.vfat -F 32 -n SLAVE_DISK bin/fat32.img

.PHONY: run
run: bin/$(OUTPUT)
	./util/run.sh $(ARCH)

.PHONY: rust-deps
rust-deps:
	rustup target add i686-unknown-linux-gnu
	rustup target add aarch64-unknown-none
	rustup target add riscv64gc-unknown-none-elf
	rustup target add wasm32-unknown-unknown
	rustup component add rust-src
	rustup component add llvm-tools-preview

.PHONY: list-arch
list-arch:
	@echo "Available architectures:"
	@ls -1 arch/

.PHONY: info
info:
	@echo "DekOS Build Information:"
	@echo "  Architecture: $(ARCH)"
	@echo "  Output: bin/$(OUTPUT)"
	@echo "  Languages: C Rust Zig Fortran"
	@echo "  C Compiler: $(KCC)"
	@echo "  Rust Compiler: $(KRUSTC)"
	@echo "  Zig Compiler: $(KZIGC)"
	@echo "  Fortran Compiler: $(KFORTRAN)"
	@echo "  Linker: $(KLD)"
	@echo ""
	@echo "Source files:"
	@echo "  C: $(words $(CFILES)) files"
	@echo "  Assembly: $(words $(ASFILES)) files"
	@echo "  NASM: $(words $(NASMFILES)) files"
	@echo "  Rust: $(words $(RUSTFILES)) files"
	@echo "  Zig: $(words $(ZIGFILES)) files"
	@echo "  Fortran: $(words $(FORFILES)) files"
