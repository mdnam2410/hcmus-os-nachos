#ifndef SEM_H
#define SEM_H
#include "synch.h"
class Semaphore;


// Class Sem use to manage Semaphore
class Sem
{
private:
	char name[50];		// Name of semaphore
	Semaphore* sem;		// Core of sem
public:
    // Constructor
    // Sem object wrap semphore object 
	Sem(char* na, int i);

	~Sem();

	void Wait();

	void Signal();
	
	char* getName();
};
#endif // SEM_H