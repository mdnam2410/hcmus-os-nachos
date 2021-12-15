#include "syscall.h"

int main()
{
    int fileId;
    fileId = Open("test1.txt", 1);
    Close(fileId);
    return 0;
}