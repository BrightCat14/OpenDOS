@echo off

set IMG_NAME=opendos.img

if not exist "%IMG_NAME%" (
    echo [91mError: %IMG_NAME% not found![0m
    echo [93mPlease build the image first using build.sh in WSL or Linux.[0m
    exit /b 1
)

echo [94mStarting QEMU with OpenDOS...[0m
qemu-system-i386 -drive file=%IMG_NAME%,format=raw -serial stdio -d int -audiodev dsound,id=ds1 -machine pcspk-audiodev=ds1

if errorlevel 1 (
    echo [93mNote: If audio doesn't work, you can run without audio:[0m
    echo [93mqemu-system-i386 -drive file=%IMG_NAME%,format=raw -serial stdio[0m
)
