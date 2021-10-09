#include "syscall.h"

int main()
{
    char c = '\0';
    c = ReadChar();
    PrintChar(c);
    return 0;
}