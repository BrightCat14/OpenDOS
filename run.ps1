# PowerShell run script for OpenDOS

$IMG_NAME = "opendos.img"

function Write-ColorOutput($ForegroundColor) {
    $fc = $host.UI.RawUI.ForegroundColor
    $host.UI.RawUI.ForegroundColor = $ForegroundColor
    if ($args) {
        Write-Output $args
    }
    $host.UI.RawUI.ForegroundColor = $fc
}

if (-not (Test-Path $IMG_NAME)) {
    Write-ColorOutput Red "Error: $IMG_NAME not found!"
    Write-ColorOutput Yellow "Please build the image first using build.sh in WSL or Linux."
    exit 1
}

Write-ColorOutput Cyan "Starting QEMU with OpenDOS..."

# Try with audio first
& qemu-system-i386 -drive file=$IMG_NAME,format=raw -serial stdio -d int -audiodev dsound,id=ds1 -machine pcspk-audiodev=ds1

if ($LASTEXITCODE -ne 0) {
    Write-ColorOutput Yellow "Note: If audio doesn't work, you can run without audio:"
    Write-ColorOutput Yellow "qemu-system-i386 -drive file=$IMG_NAME,format=raw -serial stdio"
}
