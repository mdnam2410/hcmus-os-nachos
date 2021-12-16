#include "sem.h"

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