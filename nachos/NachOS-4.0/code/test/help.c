#include "syscall.h"

int main()
{
    char* group_descriptions[] = {
        "Group members\n",
        "ID       | Name\n",
        "-------- | ----------------\n", 
        "19120298 | Mai Duy Nam\n",
        "19120491 | Dang Thai Duy\n",
        "19120690 | Vo Van Toan\n",
        "\n",
    };

    char* program_descriptions[] = {
        "Program descriptions\n",
        "ascii: Print a list of ", 
        "printable ASCII characters\n",
        "usage: ./../build.linux/nachos -x ascii\n",
        "\n",
        "sort: Sort a list of integers ",
        "using bubble sort algorithm\n",
        "usage: ./../build.linux/nachos -x sort\n",
    };

    int i;
    for (i = 0; i < 7; ++i)
        PrintString(group_descriptions[i]);

    for (i = 0; i < 8; ++i)
        PrintString(program_descriptions[i]);
    Halt();
    return 0;
}
