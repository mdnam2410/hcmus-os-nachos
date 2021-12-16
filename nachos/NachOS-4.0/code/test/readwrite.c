#include "syscall.h"
#define MAX_LENGTH 255

int main()
{
    char str[50];
    int length = 50;
    PrintString("------------CHUONG TRINH ECHO-------------\n\n");
    PrintString("Nhap chuoi: ");
    ReadString(str, length);
    PrintString("Chuoi vua nhap: ");
    PrintString(str);
    PrintString("\n\n");
    return 0;
}
