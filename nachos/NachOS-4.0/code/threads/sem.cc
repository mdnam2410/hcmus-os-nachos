#include "sem.h"

// Constructor
Sem::Sem(char* na, int i)
{
	strcpy(this->name, na);
	sem = new Semaphore(this->name, i);
}

Sem::~Sem()
{
	if(sem)
		delete sem;
}

void Sem::Wait()
{
	sem->P();	// Down(sem)
}

void Sem::Signal()
{
	sem->V();	// Up(sem)
}

char* Sem::getName()
{
	return this->name;
}