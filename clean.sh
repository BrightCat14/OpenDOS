BUILD_DIR="build"
MOUNT_POINT="img"
IMG_NAME="opendos.img"

RED='\033[31m'
GREEN='\033[32m'
BLUE='\033[34m'
PURPLE='\033[35m'
NC='\033[0m'

clean() {
    echo -e "${BLUE}=== Cleaning ===${NC}"
    sudo umount "$MOUNT_POINT" 2>/dev/null || true
    rm -rf "$BUILD_DIR" "$MOUNT_POINT" "$IMG_NAME"
}

clean
