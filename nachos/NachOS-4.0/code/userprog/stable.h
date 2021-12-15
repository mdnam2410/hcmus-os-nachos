#ifndef STABLE_H
#define STABLE_H
#include "synch.h"
#include "bitmap.h"
#define MAX_SEMAPHORE 10

class STable
{
private:
    // empty slot manager
	BitMap* bm;
    // mange maximum 10 objects Lock
	Semaphore* semTab[MAX_SEMAPHORE];
public:
    // Constructor
    // Create 10 null semephore for semTab
    // Init bm 
	STable();		

    // Destructor
	~STable();
    // Check semaphor "name" exists then create new semaphor or send error.
	int Create(char *name);

    // Check semaphor "name" exists then call this->P() to excute or send error.
	int Wait(char *name);

    // Check semaphor "name" exists then call this->V() to excute or send error.
	int Signal(char *name);
	
	// Find free slot
	int FindFreeSlot();

};
#endif