if grub-file --is-x86-multiboot bin/kernel; then
        echo multiboot confirmed
        mkdir -p isodir/boot/grub
        cp bin/kernel isodir/boot/kernel.bin
        cp grub.cfg isodir/boot/grub/grub.cfg
        sudo mount fat32.img mnt
        sudo rm -rf mnt/
        sudo cp fat32dir/* mnt/ -rf
        sudo umount mnt
        grub-mkrescue -o dekos.iso isodir
        qemu-system-x86_64 -debugcon stdio -cdrom dekos.iso -m 128 -boot d -soundhw pcspk -hdb fat32.img
else
        echo the file is not multiboot
fi