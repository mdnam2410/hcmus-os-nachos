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
    // Lock manger (10 semaphore)
	Lock* lockTab[MAX_SEMAPHORE];
public:
    // Constructor
    // Create 10 null semephore for semTab
    // Init bm 
	STable();		

    // Destructor
	~STable();
    // Check semaphor "name" exists then create new semaphor or send error.
	int Create(char *name);

    // Check semaphor "name" exists then call this->Acquire() to excute or send error.
	int Wait(char *name);

    // Check semaphor "name" exists then call this->Release() to excute or send error.
	int Signal(char *name);
	
	// Find free slot
	int FindFreeSlot();

};
#endif