#!/bin/bash

BUILD_DIR="build"
SRC_DIR="src"
KERNEL_NAME="DOSKRNL.BIN"
IMG_NAME="opendos.img"
IMG_SIZE_MB=128
MOUNT_POINT="img"
GRUB_CFG="grub.cfg"

ASM_FILES=("kernel.asm")  
C_FILES=("kernel.c" "string.c" "io.c" "ata.c" "keyboard.c" "fat12.c" "fs.c" "speaker.c" "syscall.c")    

RED='\033[31m'
GREEN='\033[32m'
BLUE='\033[34m'
PURPLE='\033[35m'
NC='\033[0m'

clean() {
    echo -e "${BLUE}=== Cleaning ===${NC}"
    rm -rf "$BUILD_DIR" "$MOUNT_POINT" "$IMG_NAME"
    mkdir -p "$BUILD_DIR" "$MOUNT_POINT"
}

compile_asm() {
    echo -e "${BLUE}=== Compiling ASM files ===${NC}"
    for asm_file in "${ASM_FILES[@]}"; do
        local base_name=$(basename "$asm_file" .asm)
        echo -n "Compiling $asm_file... "
        nasm -f elf32 "$SRC_DIR/$asm_file" -o "$BUILD_DIR/${base_name}_asm.o" -I "$SRC_DIR/include/" || { echo -e "${RED}Error!${NC}"; exit 1; }
        echo -e "${GREEN}OK${NC}"
    done
}

compile_c() {
    echo -e "${BLUE}=== Compiling C files ===${NC}"
    for c_file in "${C_FILES[@]}"; do
        local base_name=$(basename "$c_file" .c)
        echo -n "Compiling $c_file... "
        gcc -m32 -ffreestanding -nostdlib -fno-builtin -fno-stack-protector \
            -Wall -Wextra -O2 -I "$SRC_DIR/include" \
            -c "$SRC_DIR/$c_file" -o "$BUILD_DIR/${base_name}.o" || { echo -e "${RED}Error!${NC}"; exit 1; }
        echo -e "${GREEN}OK${NC}"
    done
}

link_kernel() {
    echo -e "${BLUE}=== Linking kernel ===${NC}"
    echo -n "Linking kernel... "

    local object_files=()

    for asm_file in "${ASM_FILES[@]}"; do
        local base_name=$(basename "$asm_file" .asm)
        object_files+=("$BUILD_DIR/${base_name}_asm.o")
    done

    for c_file in "${C_FILES[@]}"; do
        local base_name=$(basename "$c_file" .c)
        object_files+=("$BUILD_DIR/${base_name}.o")
    done

    ld -m elf_i386 -T "$SRC_DIR/link.ld" -o "$BUILD_DIR/$KERNEL_NAME" "${object_files[@]}" || {
        echo -e "${RED}Error!${NC}"
        exit 1
    }

    echo -e "${GREEN}OK${NC}"
}

create_image() {
    echo -e "${BLUE}=== Creating image with MBR and FAT12 partition ===${NC}"

    echo -n "Creating empty image file (${IMG_SIZE_MB}MB)... "
    dd if=/dev/zero of="$IMG_NAME" bs=1M count=$IMG_SIZE_MB status=none || { echo -e "${RED}Error!${NC}"; exit 1; }
    echo -e "${GREEN}OK${NC}"

    echo -n "Creating partition table and partition... "
	parted -s "$IMG_NAME" mklabel msdos
	parted -s "$IMG_NAME" mkpart primary 1MiB 100%
	echo "type=1" | sfdisk --part-type "$IMG_NAME" 1
    echo -e "${GREEN}OK${NC}"

    echo -n "Setting up loop device with partitions... "
    LOOP_DEV=$(sudo losetup --show -fP "$IMG_NAME") || { echo -e "${RED}Error!${NC}"; exit 1; }
    echo -e "${GREEN}OK (${LOOP_DEV})${NC}"

    echo -n "Formatting partition as FAT12... "
    sudo mkfs.fat -F 12 "${LOOP_DEV}p1" >/dev/null || { echo -e "${RED}Error!${NC}"; sudo losetup -d $LOOP_DEV; exit 1; }
    echo -e "${GREEN}OK${NC}"

    echo -n "Mounting partition... "
    sudo mount "${LOOP_DEV}p1" "$MOUNT_POINT" || { echo -e "${RED}Error!${NC}"; sudo losetup -d $LOOP_DEV; exit 1; }
    echo -e "${GREEN}OK${NC}"

    echo -n "Creating directories and copying kernel... "
    sudo mkdir -p "$MOUNT_POINT/OPENDOS" "$MOUNT_POINT/boot/grub"
    sudo cp "$BUILD_DIR/$KERNEL_NAME" "$MOUNT_POINT/OPENDOS/$KERNEL_NAME"
    echo -e "${GREEN}OK${NC}"

    echo -n "Creating grub.cfg... "
    sudo tee "$MOUNT_POINT/boot/grub/$GRUB_CFG" >/dev/null << EOF
set timeout=5
set default=0

menuentry "OpenDOS Kernel" {
    multiboot /OPENDOS/$KERNEL_NAME
    boot
}
EOF
    echo -e "${GREEN}OK${NC}"

    echo -n "Installing GRUB in MBR... "
    sudo grub-install --target=i386-pc --boot-directory="$MOUNT_POINT/boot" --modules="multiboot" --recheck --force "$LOOP_DEV" >/dev/null 2>&1 || { echo -e "${RED}Error!${NC}"; sudo umount "$MOUNT_POINT"; sudo losetup -d $LOOP_DEV; exit 1; }
    echo -e "${GREEN}OK${NC}"

    echo -n "Unmounting partition... "
    sudo umount "$MOUNT_POINT" || { echo -e "${RED}Error!${NC}"; sudo losetup -d $LOOP_DEV; exit 1; }
    echo -e "${GREEN}OK${NC}"

    echo -n "Detaching loop device... "
    sudo losetup -d "$LOOP_DEV" || { echo -e "${RED}Error!${NC}"; exit 1; }
    echo -e "${GREEN}OK${NC}"
}

main() {
    clean
    compile_asm
    compile_c
    link_kernel
    create_image
    echo -e "${PURPLE}=== Image $IMG_NAME ready! ===${NC}"
}

main "$@"
