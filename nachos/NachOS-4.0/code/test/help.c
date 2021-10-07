#include "syscall.h"

int main()
{
    char* s[] = {
        "Group members\n",
        "ID       | Name\n",
        "-------- | ----------------\n", 
        "19120298 | Mai Duy Nam\n",
        "19120690 | Vo Van Toan\n",
        "19120491 | Dang Thai Duy\n"
    };

    int i;
    for (i = 0; i < 6; ++i)
        PrintString(s[i]);
    Halt();
    return 0;
}