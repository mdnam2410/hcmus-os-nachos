// CAE - MULTI - SYNCHCONSOLE DEFINITION
#ifndef SYNCHCONS_H
#define SYNCHCONS_H

#include "callback.h"
#include "console.h"

class SynchConsole : public CallBackObj
{
public:
	SynchConsole();					   // A SynchConsole Constructor
	SynchConsole(char *in, char *out); // Same with fn pointers
	~SynchConsole();				   // Delete a console instance

	int Read(char *into, int numBytes);	 // Read synch line
										 // Ends in EOLN or ^A
	int Write(char *from, int numBytes); // Write a synchronous line

	void CallBack(); // called when a keystroke is available
private:
	ConsoleInput *consIn;	// Pointer to an async console
	ConsoleOutput *consOut; // Pointer to an async console
};

#endif
// CAE - MULTI - END SECTION
