@echo off

set BUILD_DIR=build
set IMG_NAME=opendos.img

echo [94m=== Cleaning ===[0m

if exist "%BUILD_DIR%" (
    echo Removing %BUILD_DIR%...
    rmdir /s /q "%BUILD_DIR%"
)

if exist "%IMG_NAME%" (
    echo Removing %IMG_NAME%...
    del /f /q "%IMG_NAME%"
)

echo [92mClean complete![0m
