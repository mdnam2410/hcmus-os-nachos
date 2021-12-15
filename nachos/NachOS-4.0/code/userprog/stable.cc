#include "stable.h"

// Constructor
STable::STable()
{	
	this->bm = new BitMap(MAX_SEMAPHORE);
	
	for(int i =0; i < MAX_SEMAPHORE; i++)
	{
		this->semTab[i] = NULL;
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
		if(this->semTab[i])
		{
			delete this->semTab[i];
			this->semTab[i] = NULL;
		}
	}
	
}

int STable::Create(char *name, int semVal)
{

	// Check exists semphore
	for(int i=0; i<MAX_SEMAPHORE; i++)
	{
		if(bm->Test(i))
		{
			if(strcmp(name, semTab[i]->GetName()) == 0)
			{
				return -1;
			}
		}
		
	}
	// find free slot in semTab
	int id = this->FindFreeSlot();
	
	// if semTab is full then return -1
	if(id < 0)
	{
		return -1;
	}

    // if find empty slot then load semaphore to semTab[id]
	this->semTab[id] = new Semaphore(name, semVal);
	return 0;
}

int STable::Wait(char *name)
{
	for(int i =0; i < MAX_SEMAPHORE; i++)
	{
        // Check does slot[i] load semaphore
		if(bm->Test(i))
		{
			// Neu co thi tien hanh so sanh name voi name cua semaphore trong semTab
            // if yes then compare nam with name of semaphore in semTab
			if(strcmp(name, semTab[i]->GetName()) == 0)
			{
				// Neu ton tai thi cho semaphore down(); 
				semTab[i]->Acquire();
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
		// Kiem tra o thu i da duoc nap semaphore chua
		if(bm->Test(i))
		{
			// Neu co thi tien hanh so sanh name voi name cua semaphore trong semTab
			if(strcmp(name, semTab[i]->GetName()) == 0)
			{
				// Neu ton tai thi cho semaphore up(); 
				semTab[i]->Release();
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