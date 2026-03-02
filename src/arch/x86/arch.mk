# arch/x86/arch.mk
$(info Building for architecture: x86)

override ARCH_CFLAGS += \
    -m32 \
    -march=i686 \
    -mtune=generic \
    -mno-mmx \
    -mno-sse \
    -mcmodel=kernel

override ARCH_ASFLAGS += \
    -m32 \
    --target=i686-pc-none-elf

override ARCH_KLDFLAGS += \
    -melf_i386 \
    -T src/arch/x86/linker.ld

override ARCH_KRUSTFLAGS += \
    --target i686-unknown-linux-gnu \
    -C relocation-model=static

override ARCH_KZIGFLAGS += \
    -target x86-freestanding \
    -mcpu i686

override ARCH_KFORTRANFLAGS += \
    -m32
