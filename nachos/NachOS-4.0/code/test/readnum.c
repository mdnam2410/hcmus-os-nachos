#include "syscall.h"

int main()
{
    int num;
    num = ReadNum();
    PrintString("Read: ");
    PrintNum(num);
    return 0;
}