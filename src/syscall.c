#include "os.h"
#include "syscalls.h"

uint32_t handle_syscall(uint32_t number, uint32_t arg1) {
    switch (number) {
        case SYS_INPUT:
            // TODO: return text from input
            break;

        case SYS_PRINT:
            k_printf("%s", (char*)arg1);
            break;

        case SYS_PRINTERR:
            k_printf("\x1b[31m%s\x1b[0m\n", (char*)arg1);
            break;

        case SYS_EXIT:
            // TODO: exit from program
            break;

        default:
            return -1; // unknown syscall
    }
    return 0;
}
