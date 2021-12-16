#ifndef STABLE_H
#define STABLE_H
#include "synch.h"
#include "bitmap.h"
#define MAX_SEMAPHORE 10

// Class Sem use to manage Semaphore
class Sem
{
private:
	char name[50];		// Name of semaphore
	Semaphore* sem;		// Core of sem
public:
    // Constructor
	Sem(char* na, int i)
	{
		strcpy(this->name, na);
		sem = new Semaphore(this->name, i);
	}

	~Sem()
	{
		if(sem)
			delete sem;
	}

	void wait()
	{
		sem->P();	// Down(sem)
	}

	void signal()
	{
		sem->V();	// Up(sem)
	}
	
	char* GetName()
	{
		return this->name;
	}
};

class STable
{
private:
    // Manager empty slot
	Bitmap* bm;
    // Lock manger (10 semaphore)
	Sem* table[MAX_SEMAPHORE];
public:
    // Constructor
    // Create 10 null semephore for semTab
    // Init bm 
	STable();		

    // Destructor
	~STable();
    // Check semaphor "name" exists then create new semaphor or send error.
	int Create(char *name, int value);

    // Check semaphor "name" exists then call this->Acquire() to excute or send error.
	int Wait(char *name);

    // Check semaphor "name" exists then call this->Release() to excute or send error.
	int Signal(char *name);
	
	// Find free slot
	int FindFreeSlot();

};
#endif