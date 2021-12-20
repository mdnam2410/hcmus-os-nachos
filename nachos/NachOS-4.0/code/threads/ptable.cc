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

void PTable::InitializeFirstProcess(const char *fileName, Thread* thread)
{
    _pcbs[0] = new PCB(fileName, thread);
    ++_num_processes;
    _slotManager->Mark(0);
}

int PTable::GetCurrentThreadId()
{
    for (int i = 0; i < MAX_PROCESSES; ++i) {
        if (_pcbs[i] != 0) {
            if (_pcbs[i]->GetThread() == kernel->currentThread) {
                return _pcbs[i]->GetID();
            }
        }
    }
    return -1;
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

void PTable::Remove(int pid)
{
    if (IsExist(pid)) {
        --_num_processes;
        _slotManager->Clear(pid);
        delete _pcbs[pid];
        _pcbs[pid] = NULL;
    }
}

int PTable::ExecUpdate(char *fileName)
{
    _sem->P();
    DEBUG(dbgThread, "PTable::ExecUpdate(\"" << fileName << "\")");

    // Prevent self-execution
    DEBUG(dbgThread, "PTable: Checking " << fileName << " for self-execution...");
    int currentThreadId = GetCurrentThreadId();
    if (strcmp(_pcbs[currentThreadId]->GetExecutableFileName(), fileName) == 0) {
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
    _pcbs[slot]->parentID = currentThreadId;
    
    // Schedule the program for execution
    DEBUG(dbgThread, "PTable: Scheduling program for execution...");
    int result = _pcbs[slot]->Exec(fileName, slot);
    ++_num_processes;
    _sem->V();
    return result;
}

int PTable::JoinUpdate(int id)
{
    int currentThreadId = GetCurrentThreadId();
    if (!IsExist(id)) {
        fprintf(
            stderr,
            "PTable: Join into an invalid process "
            "(there is no process with id: %d)\n",
            id
        );
        return -1;
    }

    if (id == currentThreadId) {
        fprintf(
            stderr,
            "PTable: Process with id %d cannot join to itself\n",
            currentThreadId
        );
        return -2;
    } else if (_pcbs[id]->parentID != currentThreadId) {
        fprintf(
            stderr,
            "PTable: Can only join parent to child process "
            "(process with id %d is not parent of process with id %d)\n",
            currentThreadId,
            id
        );
        return -3;
    }

    _pcbs[currentThreadId]->IncNumWait();
    _pcbs[id]->JoinWait();

    _pcbs[currentThreadId]->DecNumWait();
    _pcbs[id]->ExitRelease();
    return _pcbs[id]->GetExitCode();
}

int PTable::ExitUpdate(int ec)
{
    int currentThreadId = GetCurrentThreadId();
    if (currentThreadId == 0) {
        kernel->interrupt->Halt();
    } else {
        _pcbs[currentThreadId]->SetExitCode(ec);
        _pcbs[currentThreadId]->JoinRelease();
        _pcbs[currentThreadId]->ExitWait();
        Remove(currentThreadId);
    }
}

void PTable::Print()
{
    printf("\n\nTime: %d\n", kernel->stats->totalTicks);
    printf("Current process table:\n");
    printf("ID\tParent\tExecutable file\n");
    int currentThreadId = GetCurrentThreadId();
    for (int i = 0; i < MAX_PROCESSES; ++i) {
        if (_pcbs[i]) {
            printf(
                "%d\t%d\t%s%s\n",
                _pcbs[i]->GetID(),
                _pcbs[i]->parentID,
                _pcbs[i]->GetExecutableFileName(),
                i == currentThreadId ? " (current thread) " : ""
            );  
        }
    }
}
