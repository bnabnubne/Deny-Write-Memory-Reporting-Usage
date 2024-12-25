#include "syscall.h"
#include "stdio.h"

int main(void) {
    printf("Reporting memory usage:\n");
    syscall(SYS_REPORT_MEMORY);
    return 0;
}
