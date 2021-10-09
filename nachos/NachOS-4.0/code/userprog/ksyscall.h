/**************************************************************
 *
 * userprog/ksyscall.h
 *
 * Kernel interface for systemcalls 
 *
 * by Marcus Voelp  (c) Universitaet Karlsruhe
 *
 **************************************************************/

#ifndef __USERPROG_KSYSCALL_H__ 
#define __USERPROG_KSYSCALL_H__ 

#include "kernel.h"
// Work around to fix Nachos' crazy codebase
// 
// Due to the forward declarations of SynchConsoleInput and SynchConsoleOutput
// in threads/kernel.h, the variable synchConsoleInput and synchConsoleOutput
// aren't usable without this library included.
#include "synchconsole.h"



void SysHalt()
{
  kernel->interrupt->Halt();
}


int SysAdd(int op1, int op2)
{
  return op1 + op2;
}

// Reads at most `length` characters or until newline
// from console to `str` (null-terminated)
int SysReadString(char *str, int length) {
    int i = 0;
    char c;
    
    // Read the input from console
    do {
      c = kernel->synchConsoleIn->GetChar();
      str[i] = c;
      ++i;
    } while (i < length && c != '\n');
    str[i] = '\0';
}

// Prints at most `length` characters from `str` to console
int SysPrintString(char* str, int length) {
    for (int i = 0; i < length; ++i) {
        kernel->synchConsoleOut->PutChar(str[i]);
    }
}




#endif /* ! __USERPROG_KSYSCALL_H__ */
