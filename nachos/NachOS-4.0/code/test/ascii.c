#include "syscall.h"

int main() {
    const int START = 32;
    const int END = 127;
    
    int i = START;
    for (; i < END; ++i) {
        PrintNum(i);
        PrintChar(' ');
        PrintChar((char) i);
        PrintChar('\n');
    }
}