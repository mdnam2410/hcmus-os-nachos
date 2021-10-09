#include "syscall.h"

int main()
{
    char s[255];
    ReadString(s, 254);

    PrintString("Read: ");
    PrintString(s);
    return 0;
}
