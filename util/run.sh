if grub-file --is-x86-multiboot bin/kernel; then
        echo multiboot confirmed
        mkdir -p isodir/boot/grub
        cp bin/kernel isodir/boot/kernel.bin
        cp grub.cfg isodir/boot/grub/grub.cfg
        mkdir -p mnt
        sudo mount bin/fat32.img mnt
        sudo rm -rf mnt/*
        sudo cp fat32dir/* mnt/ -rf
        sudo umount mnt
        grub-mkrescue -o bin/dekos.iso isodir
        qemu-system-x86_64 -debugcon stdio -cdrom bin/dekos.iso -m 64 -boot d -soundhw pcspk -hdb bin/fat32.img
else
        echo the file is not multiboot
fi