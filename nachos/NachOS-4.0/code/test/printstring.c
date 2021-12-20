#include "syscall.h"

int main() {
    PrintString("printstring is starting\n");
    PrintString("hello\n");
    PrintString("printstring is exiting with exit code: 0\n");
    Exit(0);
}
