# OpenDOS Build Script for Windows (PowerShell)
#
# This script compiles the OpenDOS kernel and creates a bootable image.
# For bootable image creation, WSL (Windows Subsystem for Linux) is required.
#
# To install WSL:
#   wsl --install
#
# After installing WSL, make sure you have these packages:
#   sudo apt-get update
#   sudo apt-get install nasm gcc-multilib binutils parted grub-pc-bin
#

$BUILD_DIR = "build"
$SRC_DIR = "src"
$KERNEL_NAME = "DOSKRNL.BIN"
$IMG_NAME = "opendos.img"
$IMG_SIZE_MB = 128

function Write-ColorOutput($ForegroundColor) {
    $fc = $host.UI.RawUI.ForegroundColor
    $host.UI.RawUI.ForegroundColor = $ForegroundColor
    if ($args) {
        Write-Output $args
    }
    $host.UI.RawUI.ForegroundColor = $fc
}

Write-ColorOutput Cyan "=== Cleaning ==="
if (Test-Path $BUILD_DIR) { Remove-Item -Recurse -Force $BUILD_DIR }
if (Test-Path $IMG_NAME) { Remove-Item -Force $IMG_NAME }
New-Item -ItemType Directory -Force -Path $BUILD_DIR | Out-Null

Write-ColorOutput Cyan "=== Compiling ASM files ==="
Write-Host "Compiling kernel.asm... " -NoNewline
& nasm -f elf32 "$SRC_DIR\kernel.asm" -o "$BUILD_DIR\kernel_asm.o" -I "$SRC_DIR\include\"
if ($LASTEXITCODE -ne 0) {
    Write-ColorOutput Red "Error!"
    exit 1
}
Write-ColorOutput Green "OK"

Write-ColorOutput Cyan "=== Compiling C files ==="
$C_FILES = @("kernel.c", "string.c", "io.c", "ata.c", "keyboard.c", "fat12.c", "fs.c", "speaker.c", "syscall.c")

foreach ($c_file in $C_FILES) {
    $base_name = [System.IO.Path]::GetFileNameWithoutExtension($c_file)
    Write-Host "Compiling $c_file... " -NoNewline
    & gcc -m32 -ffreestanding -nostdlib -fno-builtin -fno-stack-protector `
        -Wall -Wextra -O2 -I "$SRC_DIR\include" `
        -c "$SRC_DIR\$c_file" -o "$BUILD_DIR\$base_name.o"
    if ($LASTEXITCODE -ne 0) {
        Write-ColorOutput Red "Error!"
        exit 1
    }
    Write-ColorOutput Green "OK"
}

Write-ColorOutput Cyan "=== Linking kernel ==="
Write-Host "Linking kernel... " -NoNewline
& ld -m elf_i386 -T "$SRC_DIR\link.ld" -o "$BUILD_DIR\$KERNEL_NAME" `
    "$BUILD_DIR\kernel_asm.o" `
    "$BUILD_DIR\kernel.o" `
    "$BUILD_DIR\string.o" `
    "$BUILD_DIR\io.o" `
    "$BUILD_DIR\ata.o" `
    "$BUILD_DIR\keyboard.o" `
    "$BUILD_DIR\fat12.o" `
    "$BUILD_DIR\fs.o" `
    "$BUILD_DIR\speaker.o" `
    "$BUILD_DIR\syscall.o"
if ($LASTEXITCODE -ne 0) {
    Write-ColorOutput Red "Error!"
    exit 1
}
Write-ColorOutput Green "OK"

Write-ColorOutput Magenta "=== Build complete! Kernel at $BUILD_DIR\$KERNEL_NAME ==="

Write-Host ""
Write-ColorOutput Cyan "=== Creating bootable image ==="

# Check if WSL is available
$wslAvailable = $null -ne (Get-Command wsl -ErrorAction SilentlyContinue)

if ($wslAvailable) {
    Write-ColorOutput Green "WSL found! Creating bootable image with GRUB..."
    Write-Host ""
    
    # Get WSL path
    $currentPath = Get-Location
    $wslPath = wsl wslpath -a $currentPath
    
    # Run image creation script in WSL
    wsl bash -c "cd '$wslPath' && ./build-image.sh"
    
    if ($LASTEXITCODE -ne 0) {
        Write-ColorOutput Red "Error creating bootable image!"
        Write-ColorOutput Yellow "You can try running build-image.sh manually in WSL."
    } else {
        Write-ColorOutput Magenta "=== Image $IMG_NAME ready! ==="
    }
} else {
    Write-ColorOutput Yellow "WSL not found! Cannot create bootable image on Windows."
    Write-ColorOutput Yellow "Options:"
    Write-ColorOutput Yellow "  1. Install WSL and run this script again"
    Write-ColorOutput Yellow "  2. Run build-image.sh manually in WSL/Linux"
    Write-ColorOutput Yellow "  3. Use the kernel binary at $BUILD_DIR\$KERNEL_NAME"
}
