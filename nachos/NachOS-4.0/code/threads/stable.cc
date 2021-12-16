#include "stable.h"

// Constructor
STable::STable()
{	
	this->bm = new Bitmap(MAX_SEMAPHORE);
	
	for(int i =0; i < MAX_SEMAPHORE; i++)
	{
		this->table[i] = NULL;
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
		if(this->table[i])
		{
			delete this->table[i];
			this->table[i] = NULL;
		}
	}
	
}

int STable::Create(char *name, int value)
{
	// Check exists semphore
	for(int i=0; i<MAX_SEMAPHORE; i++)
	{
		if(bm->Test(i))
		{
			if(strcmp(name, table[i]->GetName()) == 0)
			{
				return -1;
			}
		}
		
	}
	// Find free slot in table
	int id = this->FindFreeSlot();
	
	// If table is full then return -1
	if (id < 0)
	{
		return -1;
	}

    // If find empty slot then load semaphore to table[id]
	this->table[id] = new Sem(name, value);
	return 0;
}

int STable::Wait(char *name)
{
	for(int i =0; i < MAX_SEMAPHORE; i++)
	{
        // Check does slot[i] load semaphore
		if(bm->Test(i))
		{
            // if yes then compare nam with name of semaphore in table
			if(strcmp(name, table[i]->GetName()) == 0)
			{
                // If exist then make semaphore down()
				table[i]->wait();
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
            // if yes then compare nam with name of semaphore in table
			if(strcmp(name, table[i]->GetName()) == 0)
			{
                // If exist then make semaphore up()
				table[i]->signal();
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