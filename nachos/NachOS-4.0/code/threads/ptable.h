#ifndef PTABLE_H
#define PTABLE_H

#include "pcb.h"
#include "bitmap.h"
#include "synch.h"
#include "thread.h"

#define MAX_PROCESSES 10
class PCB;
class Semaphore;
class PTable {
public:
    // Initializes process table with `size` entries
    PTable(int size = MAX_PROCESSES);

    ~PTable();

    // Allocates the start up process (first executed by Nachos)
    void InitializeFirstProcess(const char* fileName, Thread* thread);

    int GetCurrentThreadId();

    // Sets up PCB and schedules the program stored in `fileName`.
    // Returns the process id on success, -1 otherwise
    int ExecUpdate(char *fileName);

    // Updates the process table when the current thread join into the thread
    // `id`
    int JoinUpdate(int id);

    // Updates the process table when the current thread exit with exit code
    // `ec`
    int ExitUpdate(int ec);

    // Return an index to a free slot in the table, returns -1
    // if the table is full
    int GetFreeSlot();

    // Returns TRUE if pid exists
    bool IsExist(int pid);

    // Removes a PCB
    void Remove(int pid);

    // Print the current process table
    void Print();

private:
    // The array of system-wide PCBs to manage
    PCB *_pcbs[MAX_PROCESSES];

    // Number of alive processes in the system
    int _num_processes;

    // Manages free slots in the PCBs table
    Bitmap *_slotManager;

    // 
    Semaphore *_sem;
};


#endif // PTABLE_H
