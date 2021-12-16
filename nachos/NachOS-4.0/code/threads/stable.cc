#include "stable.h"

// Constructor
STable::STable()
{	
	this->bm = new BitMap(MAX_SEMAPHORE);
	
	for(int i =0; i < MAX_SEMAPHORE; i++)
	{
		this->lockTab[i] = NULL;
	}
}

// Destructor
STable::~STable()
{
	if(this->bm)
	{
		delete this->bm;
		this->bm = NULL;
	}
	for(int i=0; i < MAX_SEMAPHORE; i++)
	{
		if(this->lockTab[i])
		{
			delete this->lockTab[i];
			this->lockTab[i] = NULL;
		}
	}
	
}

int STable::Create(char *name)
{

	// Check exists semphore
	for(int i=0; i<MAX_SEMAPHORE; i++)
	{
		if(bm->Test(i))
		{
			if(strcmp(name, lockTab[i]->getName()) == 0)
			{
				return -1;
			}
		}
		
	}
	// Find free slot in lockTab
	int id = this->FindFreeSlot();
	
	// If lockTab is full then return -1
	if(id < 0)
	{
		return -1;
	}

    // If find empty slot then load semaphore to lockTab[id]
	this->lockTab[id] = new Lock(name);
	return 0;
}

int STable::Wait(char *name)
{
	for(int i =0; i < MAX_SEMAPHORE; i++)
	{
        // Check does slot[i] load semaphore
		if(bm->Test(i))
		{
            // if yes then compare nam with name of semaphore in lockTab
			if(strcmp(name, lockTab[i]->getName()) == 0)
			{
                // If exist then make semaphore down()
				lockTab[i]->Acquire();
				return 0;
			}
		}
	}
	printf("Not exists semaphore");
	return -1;
}

int STable::Signal(char *name)
{
	for(int i =0; i < MAX_SEMAPHORE; i++)
	{
        // Check does slot[i] load semaphore
		if(bm->Test(i))
		{
            // if yes then compare nam with name of semaphore in lockTab
			if(strcmp(name, lockTab[i]->getName()) == 0)
			{
                // If exist then make semaphore up()
				lockTab[i]->Release();
				return 0;
			}
		}
	}
	printf("Not exists semaphore");
	return -1;
}

int STable::FindFreeSlot()
{
	return this->bm->FindAndSet();
}