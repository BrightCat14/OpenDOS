#define SYS_INPUT  0
#define SYS_PRINT  1
#define SYS_PRINTERR  2
#define SYS_EXIT   5

/*
// future use:
static inline uint32_t syscall(uint32_t number, uint32_t arg1, uint32_t arg2) {
    uint32_t ret;
    asm volatile (
        "int $0x80" 
        : "=a"(ret)
        : "a"(number), "b"(arg1), "c"(arg2)
    );
    return ret;
}

void hello() {
    syscall(SYS_PRINT, (uint32_t)"Hello from user space!\n", 0);
}
*/
