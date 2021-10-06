#include "syscall.h"

int main()
{
    printf("hllo");
    char c = ReadChar();
    PrintChar(c);
    PrintChar(c);
    Halt();
    return 0;
}