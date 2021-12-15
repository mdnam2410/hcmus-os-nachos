#ifndef STABLE_H
#define STABLE_H
#include "synch.h"
#include "bitmap.h"
#define MAX_SEMAPHORE 10

class STable
{
private:
    // Manager empty slot
	BitMap* bm;
    // Semaphore manger (10 semaphore)
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