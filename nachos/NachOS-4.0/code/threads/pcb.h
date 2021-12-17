#ifndef PCB_H
#define PCB_H

#include "synch.h"
#include "thread.h"

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

    // Creates thread and schedules for program stored in `fileName`
    //
    // Returns the process ID on success, -1 on failure
    int Exec(char *fileName, int pid);

    int GetID();

    int GetNumWait();

    void JoinWait();

    void ExitWait();

    void JoinRelease();

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
