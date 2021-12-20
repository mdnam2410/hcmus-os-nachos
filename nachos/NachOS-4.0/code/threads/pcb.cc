#include "pcb.h"

#include "addrspace.h"
#include "main.h"
#include "synch.h"
#include "thread.h"

#include <stdio.h>

// Entry point for executing processes
static void StartProcess(void *args)
{
    char *fileName = (char*) args;
    DEBUG(dbgThread, "PCB: Starting process " << fileName);
    AddrSpace *addrspace = new AddrSpace;
    if (addrspace->Load(fileName)) {
        addrspace->Execute();
    }
    ASSERTNOTREACHED();
}


PCB::PCB()
{
    _pid = -1;
    parentID = -1;
    _numwait = 0;
    _exitCode = 0;
    _file = NULL;
    _thread = NULL;
    _joinsem = new Semaphore("joinsem", 0);
    _exitsem = new Semaphore("exitsem", 0);
    _mutex = new Semaphore("mutex", 1);
}

PCB::PCB(const char *fileName, Thread *thread)
    : PCB()
{
    _pid = 0;
    _file = new char[strlen(fileName) + 1];
    strcpy(_file, fileName);
    _thread = thread;
}

PCB::~PCB()
{
    delete _file;
    if (_thread) {
        _thread->Finish();
        delete _thread;
    }
    delete _joinsem;
    delete _exitsem;
    delete _mutex;
}

int PCB::GetID()
{
    return _pid;
}

const char* PCB::GetExecutableFileName()
{
    return _file;
}

const Thread* PCB::GetThread()
{
    return _thread;
}

int PCB::GetNumWait()
{
    return _numwait;
}

int PCB::GetExitCode()
{
    return _exitCode;
}

void PCB::SetExitCode(int exitCode)
{
    _exitCode = exitCode;
}

void PCB::IncNumWait()
{
    _mutex->P();
    ++_numwait;
    _mutex->V();
}

void PCB::DecNumWait()
{
    _mutex->P();
    --_numwait;
    _mutex->V();
}

int PCB::Exec(char *fileName, int pid)
{
    _mutex->P();
    DEBUG(dbgThread, "PCB: Setting things up for " << fileName << "...");

    _pid = pid;

    // Copy the executable file name into local storage, since `fileName`
    // is going to be reused elsewhere
    _file = new char[strlen(fileName)];
    strcpy(_file, fileName);

    DEBUG(dbgThread, "PCB: Forking " << _file << "...");
    _thread = new Thread(_file);
    _thread->Fork(StartProcess, _file);
    _mutex->V();
    return _pid;
}

void PCB::JoinWait()
{
    _joinsem->P();
}

void PCB::ExitWait()
{
    _exitsem->P();
}

void PCB::JoinRelease()
{
    _joinsem->V();
}

void PCB::ExitRelease()
{
    _exitsem->V();
}
