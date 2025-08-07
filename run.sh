qemu-system-i386 -drive file=opendos.img,format=raw -serial stdio -d int -audiodev pa,id=pa1 -machine pcspk-audiodev=pa1
