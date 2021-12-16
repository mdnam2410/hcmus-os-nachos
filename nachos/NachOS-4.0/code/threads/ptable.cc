#include "ptable.h"

#include "bitmap.h"
#include "synch.h"

#include <stdio.h>

PTable::PTable(int size)
{
    _num_processes = size;
    _slotManager = new Bitmap(_num_processes);
    _sem = new Semaphore("ptable_bmsem", 1);

    for (int i = 0; i < MAX_PROCESSES; ++i) {
        _pcbs[i] = NULL;
    }
}

PTable::~PTable()
{
    delete _slotManager;
    delete _sem;
    for (int i = 0; i < _num_processes; ++i) {
        delete _pcbs[i];
    }
}

void PTable::InitializeFirstProcess(Thread* thread)
{
    _pcbs[0] = new PCB;
}

int PTable::GetFreeSlot()
{
    return _slotManager->FindAndSet();
}

bool PTable::IsExist(int pid)
{
    if (pid < 0 || pid >= MAX_PROCESSES) {
        return FALSE;
    }

    return _pcbs[pid] != NULL;
}

int PTable::ExecUpdate(char *fileName)
{
    _sem->P();
    DEBUG(dbgThread, "PTable::ExecUpdate(\"" << fileName << "\")");

    // Prevent self-execution
    DEBUG(dbgThread, "PTable: Checking " << fileName << " for self-execution...")
    if (strcmp(kernel->currentThread->getName(), fileName) == 0) {
        fprintf(stderr, "PTable: %s cannot execute itself.\n", fileName);
        _sem->V();
        return -1;
    }

    // Allocate a new PCB
    DEBUG(dbgThread, "PTable: Looking for free slot in process table...");
    int slot = GetFreeSlot();
    if (slot == -1) {
        fprintf(stderr, "PTable: Maximum number of processes reached.\n");
        _sem->V();
        return -1;
    }
    DEBUG(dbgThread, "PTable: Obtained slot " << slot);
    // PID = slot number
    _pcbs[slot] = new PCB();
    
    // Schedule the program for execution
    DEBUG(dbgThread, "PTable: Scheduling program for execution...");
    int result = _pcbs[slot]->Exec(fileName, slot);
    ++_num_processes;
    _sem->V();
    return result;
}
