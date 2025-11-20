@echo off
REM OpenDOS Build Script for Windows
REM 
REM This script compiles the OpenDOS kernel and creates a bootable image.
REM For bootable image creation, WSL (Windows Subsystem for Linux) is required.
REM 
REM To install WSL:
REM   wsl --install
REM 
REM After installing WSL, make sure you have these packages:
REM   sudo apt-get update
REM   sudo apt-get install nasm gcc-multilib binutils parted grub-pc-bin
REM

setlocal enabledelayedexpansion

set BUILD_DIR=build
set SRC_DIR=src
set KERNEL_NAME=DOSKRNL.BIN
set IMG_NAME=opendos.img
set IMG_SIZE_MB=128

echo [94m=== Cleaning ===[0m
if exist "%BUILD_DIR%" rmdir /s /q "%BUILD_DIR%"
if exist "%IMG_NAME%" del /f /q "%IMG_NAME%"
mkdir "%BUILD_DIR%"

echo [94m=== Compiling ASM files ===[0m
echo Compiling kernel.asm...
nasm -f elf32 "%SRC_DIR%\kernel.asm" -o "%BUILD_DIR%\kernel_asm.o" -I "%SRC_DIR%\include\"
if errorlevel 1 (
    echo [91mError compiling kernel.asm[0m
    exit /b 1
)
echo [92mOK[0m

echo [94m=== Compiling C files ===[0m
set C_FILES=kernel.c string.c io.c ata.c keyboard.c fat12.c fs.c speaker.c syscall.c

for %%f in (%C_FILES%) do (
    echo Compiling %%f...
    gcc -m32 -ffreestanding -nostdlib -fno-builtin -fno-stack-protector -Wall -Wextra -O2 -I "%SRC_DIR%\include" -c "%SRC_DIR%\%%f" -o "%BUILD_DIR%\%%~nf.o"
    if errorlevel 1 (
        echo [91mError compiling %%f[0m
        exit /b 1
    )
    echo [92mOK[0m
)

echo [94m=== Linking kernel ===[0m
echo Linking kernel...
ld -m elf_i386 -T "%SRC_DIR%\link.ld" -o "%BUILD_DIR%\%KERNEL_NAME%" "%BUILD_DIR%\kernel_asm.o" "%BUILD_DIR%\kernel.o" "%BUILD_DIR%\string.o" "%BUILD_DIR%\io.o" "%BUILD_DIR%\ata.o" "%BUILD_DIR%\keyboard.o" "%BUILD_DIR%\fat12.o" "%BUILD_DIR%\fs.o" "%BUILD_DIR%\speaker.o" "%BUILD_DIR%\syscall.o"
if errorlevel 1 (
    echo [91mError linking kernel[0m
    exit /b 1
)
echo [92mOK[0m

echo [95m=== Build complete! Kernel at %BUILD_DIR%\%KERNEL_NAME% ===[0m

echo.
echo [96m=== Creating bootable image ===[0m

REM Check if WSL is available
where wsl >nul 2>nul
if %errorlevel% equ 0 (
    echo [92mWSL found! Creating bootable image with GRUB...[0m
    echo.
    
    REM Convert Windows path to WSL path and run the image creation part
    for /f "tokens=*" %%i in ('wsl wslpath -a "%CD%"') do set WSL_PATH=%%i
    
    wsl bash -c "cd '!WSL_PATH!' && ./build-image.sh"
    
    if errorlevel 1 (
        echo [91mError creating bootable image![0m
        echo [93mYou can try running build-image.sh manually in WSL.[0m
    ) else (
        echo [95m=== Image %IMG_NAME% ready! ===[0m
    )
) else (
    echo [93mWSL not found! Cannot create bootable image on Windows.[0m
    echo [93mOptions:[0m
    echo [93m  1. Install WSL and run this script again[0m
    echo [93m  2. Run build-image.sh manually in WSL/Linux[0m
    echo [93m  3. Use the kernel binary at %BUILD_DIR%\%KERNEL_NAME%[0m
)

endlocal
