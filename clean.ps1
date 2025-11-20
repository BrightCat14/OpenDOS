# PowerShell clean script for OpenDOS

$BUILD_DIR = "build"
$IMG_NAME = "opendos.img"

function Write-ColorOutput($ForegroundColor) {
    $fc = $host.UI.RawUI.ForegroundColor
    $host.UI.RawUI.ForegroundColor = $ForegroundColor
    if ($args) {
        Write-Output $args
    }
    $host.UI.RawUI.ForegroundColor = $fc
}

Write-ColorOutput Cyan "=== Cleaning ==="

if (Test-Path $BUILD_DIR) {
    Write-Host "Removing $BUILD_DIR..."
    Remove-Item -Recurse -Force $BUILD_DIR
}

if (Test-Path $IMG_NAME) {
    Write-Host "Removing $IMG_NAME..."
    Remove-Item -Force $IMG_NAME
}

Write-ColorOutput Green "Clean complete!"
