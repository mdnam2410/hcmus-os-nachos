#ifndef PCB_H
#define PCB_H

#include "synch.h"
#include "thread.h"
class Semaphore;
// The process control block
//
// This class stores and manages information necessary to execute and join
// a process, including:
// - Process ID
// - Parent ID
// - The executable file containing the program
// - The thread used to execute the program
// - Information for managing joining operation 
//
// Instances of this class are entries in the system-wide process table,
// manage by PTable
class PCB {
public:
    int parentID;

    // Creates a new PCB
    PCB();

    ~PCB();

    // Initialize the first PCB
    //
    // This constructor should be used exclusively for the first program 
    // executed by Nachos. Because we are now supporting process control
    // blocks for Nachos, so it's obvious that the first process should be the
    // first one to be allocated to the process table.
    PCB(const char *fileName, Thread *thread);

    // Creates thread and schedules for program stored in `fileName`
    //
    // Returns the process ID on success, -1 on failure
    int Exec(char *fileName, int pid);

    int GetID();

    // Returns the name of the file used to run this process.
    const char *GetExecutableFileName();

    // Returns the current Thread object of this process. Use with care.
    const Thread* GetThread();

    // Returns the number of processes this process is waiting for.
    int GetNumWait();

    // Blocks until JoinRelease() is called. This method is called by the 
    // parent process that wants to join into this process (i.e. waiting for
    // this process to finish).
    void JoinWait();

    // Releases the parent that are waiting for this process.
    void JoinRelease();

    void ExitWait();

    void ExitRelease();

    void IncNumWait();

    void DecNumWait();

    void SetExitCode(int exitCode);

    int GetExitCode();

private:
    int _pid;

    int _exitCode;

    // Name of the executable file to load and run
    char *_file;

    // The thread object to be run
    Thread *_thread;

    // Number of joined processes
    int _numwait;

    // Semaphore to use for controlling join process
    Semaphore *_joinsem;

    // Semaphore to use for controlling exit process
    Semaphore *_exitsem;

    Semaphore *_mutex;
};

#endif // PCB_H
