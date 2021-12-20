#include "syscall.h"

int main() {
    int id = 0;
    int ec = 0;
    
    PrintString("join is executing printstring\n");
    id = Exec("printstring");
    PrintString("join is joining into printstring\n");
    ec = Join(id);
    PrintString("join has returned\n");
    PrintString("join receive exit code from printstring: ");
    PrintNum(ec);
    PrintChar('\n');
}