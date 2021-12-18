// exception.cc
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "main.h"
#include "syscall.h"
#include "ksyscall.h"
#include "synchconsole.h"
#include "stdint.h"
#include "time.h"
#include "stable.h"
#include "ptable.h"

#define MaxFileLength 32 // Maximum length of a file name
//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2.
//
// If you are handling a system call, don't forget to increment the pc
// before returning. (Or else you'll loop making the same system call forever!)
//
//	"which" is the kind of exception.  The list of possible exceptions
//	is in machine.h.
//----------------------------------------------------------------------

// Update current Program counter to the next Program counter
void IncreasePC()
{
	// read current PC and set value of it to counter
	int counter = kernel->machine->ReadRegister(PCReg);
	// update previous PC = counter
	kernel->machine->WriteRegister(PrevPCReg, counter);
	// read next PC and set value of it to counter
	counter = kernel->machine->ReadRegister(NextPCReg);
	// update current PC = counter
	kernel->machine->WriteRegister(PCReg, counter);
	// update next PC = counter + 4 (all instructions are 4 bytes wide)
	kernel->machine->WriteRegister(NextPCReg, counter + 4);
}

// Input: User space address(int) - limit of buffer (int)
// Output: cache Buffer (char*)
// Usage: Copy user memory space to system memory space
char *User2System(int virtAddr, int limit)
{
	int i; // Index
	int oneChar;
	char *kernelBuf = NULL;
	kernelBuf = new char[limit + 1]; // need for terminal
	if (kernelBuf == NULL)
		return kernelBuf;

	memset(kernelBuf, 0, limit + 1);

	for (i = 0; i < limit; i++)
	{
		kernel->machine->ReadMem(virtAddr + i, 1, &oneChar);
		kernelBuf[i] = (char)oneChar;
		if (oneChar == 0)
			break;
	}
	return kernelBuf;
}

// Input: User memory space (int) - limit of buffer (int) - cache of buffer (char*)
// Output: number of bytes copied
// Usage: Copy System memory space to User memory space
int System2User(int virtAddr, int len, char *buffer)
{
	if (len < 0)
		return -1;
	if (len == 0)
		return len;
	int i = 0;
	int oneChar = 0;
	do
	{
		oneChar = (int)buffer[i];
		kernel->machine->WriteMem(virtAddr + i, 1, oneChar);
		i++;
	} while (i < len && oneChar != 0);
	return i;
}

// Input: None
// Output: Return an Integer from console
// Usage: Read an Integer from console
void ExceptionHandlerReadNum()
{
	// int: [-2147483648 , 2147483647] --> max length = 10 not count char '-'
	// Handle to not overflow int by not to allow system read more than length of int
	const int MAX_BUFFER = 255;
	char *num_buffer = new char[MAX_BUFFER + 1];
	bool isPositive = true;
	bool isNaN = false;
	long long res = 0;

	int i = 0;
	for (; i < MAX_BUFFER; i++)
	{
		// get each character to check
		char c = kernel->synchConsoleIn->GetChar();

		// valid number
		if (c >= '0' && c <= '9')
			num_buffer[i] = c;

		// negative number
		// code like this only can handle case -{number} and {number}
		// case {array of + and -}{number} can handle
		else if (i == 0 && c == '-')
		{
			isPositive = false;
			i--;
		}

		// when user enter after input number -> out the input
		else if (c == '\n')
		{
			break;
		}

		// NaN: not a number
		else
		{
			isNaN = true;
		}
	}

	int length_num = i; // length of number_buffer use for number

	// if input is only '-' or not a number then we throw error
	if ((!isPositive && length_num == 0) || isNaN)
	{
		printf("\n\nThe integer number is not valid");
		DEBUG('a', "\nThe integer number is not valid");
		kernel->machine->WriteRegister(2, 0);
		delete num_buffer;
		return;
	}

	//	convert num_buffer into a number
	for (int i = 0; i < length_num; i++)
	{
		if (num_buffer[i] >= '0' && num_buffer[i] <= '9')
			res = res * 10 + (int)(num_buffer[i] - '0');
	}

	// check valid with integer 32 bit limit
	if (res > INT32_MAX || res < INT32_MIN)
	{
		printf("\n\nThe integer number is out of range");
		DEBUG('a', "\nThe integer number is out of range");
		kernel->machine->WriteRegister(2, 0);
		delete num_buffer;
		return;
	}

	// if res is negative, then opposite ret; else keep the result
	res = (!isPositive) ? (-res) : res;

	// write result res into kernel
	kernel->machine->WriteRegister(2, (int)res);
}

// Input: an Integer
// Output: None
// Usage: print an Integer on console
void ExceptionHandlerPrintNum()
{
	// read number from register 4
	int number = kernel->machine->ReadRegister(4);

	/*int: [-2147483648 , 2147483647] --> max buffer = 11*/
	const int MAX_BUFFER = 11;
	char *num_buffer = new char[MAX_BUFFER];

	// make a temp array full with 0
	int temp[MAX_BUFFER] = {0};

	// index counter
	int i, j;
	i = j = 0;

	bool isPositive = true;

	// negative number
	if (number < 0)
	{
		number = -number;
		num_buffer[i] = '-';
		i++;
		isPositive = false;
	}

	// save each num in number from end to start into temp array
	do
	{
		temp[j] = number % 10;
		number /= 10;
		j++;
	} while (number);

	int length = isPositive ? j : j + 1; // real buffer size for number

	while (j)
	{
		j--;
		num_buffer[i] = '0' + (char)temp[j];
		i++;
	}

	// print the result to console
	for (int i = 0; i < length; i++)
		kernel->synchConsoleOut->PutChar(num_buffer[i]);
}

// Input: None
// Output: return an positive integer
// Usage: create and return a random number
void ExceptionHandlerRandomNum()
{
	srand(time(NULL));

	// random positive number
	int number;
	do
	{
		// random an positive integer number
		number = rand();
	} while (number == 0);

	// write number to kernel
	kernel->machine->WriteRegister(2, number);
}

// Usage: Read a string from console to user space
// Input: The buffer's address
// Output: None
void ExceptionHandlerReadString()
{
	// Retrieve buffer's address
	int addr = kernel->machine->ReadRegister(4);
	int length = kernel->machine->ReadRegister(5);

	char *s = new char[length + 1];
	SysReadString(s, length);

	// Place the input from kernel space to user space
	System2User(addr, length + 1, s);
	delete s;
}

// Usage: Print a string to the console
// Input: Starting address of the string
// Output: None
void ExceptionHandlerPrintString()
{
	// Retrieve the string address in user space
	int addr = kernel->machine->ReadRegister(4);

	// Copy the string into kernel space
	char *buffer;
	buffer = User2System(addr, 255);

	// Find the string's length
	int len = 0;
	while (buffer[len] != '\0' && len < 255)
		len++;

	// Print the string to console
	SysPrintString(buffer, len);
	delete buffer;
}

// Usage: Read a char to the console
// Input: None
// Output: None
// Only return the first char of string input
void ExceptionHandlerReadChar()
{
	// Retrieve buffer's address
	int length = 255;

	char *s = new char[length + 1];
	SysReadString(s, length);
	char c = s[0];
	delete s;
	kernel->machine->WriteRegister(2, c);
}

// Usage: Print a char to the console
// Input: A char
// Output: None
void ExceptionHandlerPrintChar()
{
	char c = (char)kernel->machine->ReadRegister(4);
	kernel->synchConsoleOut->PutChar(c);
}

// Usage: Create a file with filename
// Input: address file from user space
// Output: 0: success, -1: fail
void ExceptionHandlerCreateFile()
{
	int virtAddr;
	char *filename;

	// read filename from register r4
	virtAddr = kernel->machine->ReadRegister(4);
	// copy from user space to kernel space
	filename = User2System(virtAddr, MaxFileLength + 1);

	// filename len = 0
	if (strlen(filename) == 0)
	{
		printf("\n File name is not valid");
		DEBUG('a', "\n File name is not valid");
		kernel->machine->WriteRegister(2, -1); // fail
		delete[] filename;
		return;
	}

	// can't read filename
	if (filename == NULL)
	{
		printf("\n Not enough memory in system");
		DEBUG('a', "\n Not enough memory in system");
		kernel->machine->WriteRegister(2, -1); //  fail
		delete[] filename;
		return;
	}
	DEBUG('a', "\n Finish reading filename.");

	// fail in create file
	if (!kernel->fileSystem->Create(filename, 0))
	{
		printf("\n Error create file '%s'", filename);
		DEBUG('a', "\n Error create file '%s'" << filename);
		kernel->machine->WriteRegister(2, -1); // fail
		delete[] filename;
		return;
	}

	kernel->machine->WriteRegister(2, 0); // success
	delete[] filename;
}

// Usage: Open a file
// Input: arg1: name address, arg2: type
// type: 0: read and write, 1: read only, 2: stdin, 3: stdout
// Output: return OpenFileId, if fail then return -1
void ExceptionHandlerOpen()
{
	// save
	// OpenFileID Open(char *name, int type)
	int virtAddr;
	int type;
	char *filename;
	int freeSlot;

	virtAddr = kernel->machine->ReadRegister(4); // read name address from 4th register
	type = kernel->machine->ReadRegister(5);	 // read type from 5th register

	filename = User2System(virtAddr, MaxFileLength); // Copy filename charArray form userSpace to systemSpace

	// Check if OS can still open file or not
	freeSlot = kernel->fileSystem->FindFreeSlot();
	if (freeSlot == -1) // no free slot found
	{
		printf("\n Full slot in openTable");
		DEBUG('a', "\n Full slot in openTable");
		kernel->machine->WriteRegister(2, -1); // write -1 to register r2
		delete[] filename;
		return;
	}

	// Check each type of open file
	switch (type)
	{
	case 0: //  Read and write
	case 1: //  Read only
		if ((kernel->fileSystem->openTable[freeSlot] = kernel->fileSystem->Open(filename, type)) != NULL)
		{
			kernel->machine->WriteRegister(2, freeSlot); // success -> write OpenFileID to register r2
		}
		else
		{
			printf("\n File does not exist");
			DEBUG('a', "\n File does not exist");
			kernel->machine->WriteRegister(2, -1); // fail
		}
		break;
	case 2:									  //  stdin - read from console
		kernel->machine->WriteRegister(2, 0); // stdin have OpenFileID 0
		break;
	case 3:									  //  stdout - write to console
		kernel->machine->WriteRegister(2, 1); // stdout have OpenFileID 1
		break;
	default:
		printf("\nType parameter is not match");
		DEBUG('a', "\nType parameter is not match");
		kernel->machine->WriteRegister(2, -1); // fail
	}
	delete[] filename;
}

// Usage: Close a file
// Input :  id cua file (OpenFileId)
// Output : success: 0, fail: -1
void ExceptionHandlerClose()
{
	int fileID;

	fileID = kernel->machine->ReadRegister(4); // read fileID from register r4
	if (fileID >= 0 && fileID < 10)
	{
		if (kernel->fileSystem->openTable[fileID])
		{
			delete kernel->fileSystem->openTable[fileID]; // delete file space
			kernel->fileSystem->openTable[fileID] = NULL;
			kernel->machine->WriteRegister(2, 0); // success
			return;
		}
		else
		{
			printf("\n File was not opened");
			DEBUG('a', "\n File was not opened");
			kernel->machine->WriteRegister(2, -1); // fail
			return;
		}
	}

	printf("\n FileID is not match");
	DEBUG('a', "\n FileID is not match");
	kernel->machine->WriteRegister(2, -1); // fail
}

void ExceptionHandlerRead()
{
}

void ExceptionHandlerWrite()
{
}

void ExceptionHandlerExec()
{
	DEBUG(dbgSys, "Syscall: Exec(filename)");

	int addr = kernel->machine->ReadRegister(4);
	DEBUG(dbgSys, "Register 4: " << addr);

	char *fileName;
	fileName = User2System(addr, 255);
	DEBUG(dbgSys, "Read file name: " << fileName);

	DEBUG(dbgSys, "Scheduling execution...");
	int result = pTab->ExecUpdate(fileName);

	DEBUG(dbgSys, "Writing result to register 2: " << result);
	kernel->machine->WriteRegister(2, result);
	delete fileName;
}

void ExceptionHandlerJoin()
{
	DEBUG(dbgSys, "Syscall: Join");
	int id = kernel->machine->ReadRegister(4);
	int result = pTab->JoinUpdate(id);
	kernel->machine->WriteRegister(2, result);
}

void ExceptionHandlerExit()
{
	DEBUG(dbgSys, "Syscall: Exit");
	int exitCode = kernel->machine->ReadRegister(4);
	int result = pTab->ExitUpdate(exitCode);
}

// Usage: Create a semaphore
// Input : name of semphore and int for semaphore value
// Output : success: 0, fail: -1
void ExceptionHandlerCreateSemaphore()
{
	// Load name and value of semaphore
	int virtAddr = kernel->machine->ReadRegister(4); // read name address from 4th register
	int semVal = kernel->machine->ReadRegister(5);	 // read type from 5th register
	char *name = User2System(virtAddr, MaxFileLength); // Copy semaphore name charArray form userSpace to systemSpace

	// Validate name
	if(name == NULL)
	{
		DEBUG('a', "\n Not enough memory in System");
		printf("\n Not enough memory in System");
		kernel->machine->WriteRegister(2, -1);
		delete[] name;
		return;
	}
	
	int res = semTab->Create(name, semVal);

	// Check error
	if(res == -1)
	{
		DEBUG('a', "\n Can not create semaphore");
		printf("\n Can not create semaphore");
	}
	
	delete[] name;
	kernel->machine->WriteRegister(2, res);
	return;
}

// Usage: Sleep
// Input : name of semaphore
// Output : success: 0, fail: -1
void ExceptionHandlerWait()
{
	// Load name of semaphore
	int virtAddr = kernel->machine->ReadRegister(4);
	char *name = User2System(virtAddr, MaxFileLength + 1);

	// Validate name
	if(name == NULL)
	{
		DEBUG('a', "\n Not enough memory in System");
		printf("\n Not enough memory in System");
		kernel->machine->WriteRegister(2, -1);
		delete[] name;
		return;
	}

	int res = semTab->Wait(name);
	
	// Check error
	if(res == -1)
	{
		DEBUG('a', "\n Not exists semaphore");
		printf("\n Not exists semaphore");
	}

	delete[] name;
	kernel->machine->WriteRegister(2, res);
	return;
}

// Usage: Wake up
// Input : name of semaphore
// Output : success: 0, fail: -1
void ExceptionHandlerSignal()
{
	// Load name of semphore
	int virtAddr = kernel->machine->ReadRegister(4);
	char *name = User2System(virtAddr, MaxFileLength + 1);

	// Validate name
	if(name == NULL)
	{
		DEBUG('a', "\n Not enough memory in System");
		printf("\n Not enough memory in System");
		kernel->machine->WriteRegister(2, -1);
		delete[] name;
		return;
	}
	
	int res = semTab->Signal(name);

	// Check error
	if(res == -1)
	{
		DEBUG('a', "\n Not exists semaphore");
		printf("\n Not exists semaphore");
	}
	
	delete[] name;
	kernel->machine->WriteRegister(2, res);
	return;
}

void ExceptionHandler(ExceptionType which)
{
	int type = kernel->machine->ReadRegister(2);

	switch (which)
	{
	case NoException:
		return;
	case PageFaultException:
		DEBUG('a', "\n Unexpected user mode exception PageFaultException");
		printf("\n\n Unexpected user mode exception PageFaultException");
		kernel->interrupt->Halt();
		break;

	case ReadOnlyException:
		DEBUG('a', "\n Unexpected user mode exception ReadOnlyException");
		printf("\n\n Unexpected user mode exception ReadOnlyException");
		kernel->interrupt->Halt();
		break;

	case BusErrorException:
		DEBUG('a', "\n Unexpected user mode exception BusErrorException");
		printf("\n\n Unexpected user mode exception BusErrorException");
		kernel->interrupt->Halt();
		break;

	case AddressErrorException:
		DEBUG('a', "\n Unexpected user mode exception AddressErrorException");
		printf("\n\n Unexpected user mode exception AddressErrorException");
		kernel->interrupt->Halt();
		break;

	case OverflowException:
		DEBUG('a', "\n Unexpected user mode exception OverflowException");
		printf("\n\n Unexpected user mode exception OverflowException");
		kernel->interrupt->Halt();
		break;

	case IllegalInstrException:
		DEBUG('a', "\n Unexpected user mode exception IllegalInstrException");
		printf("\n\n Unexpected user mode exception IllegalInstrException");
		kernel->interrupt->Halt();
		break;

	case NumExceptionTypes:
		DEBUG('a', "\n Unexpected user mode exception NumExceptionTypes");
		printf("\n\n Unexpected user mode exception NumExceptionTypes");
		kernel->interrupt->Halt();
		break;

	case SyscallException:
		switch (type)
		{
		case SC_Halt:
			// Input: None
			// Output: Shutdown notification
			// Usage: Shutdown

			DEBUG('a', "\nShutdown, initiated by user program. ");
			printf("\nShutdown, initiated by user program. ");
			kernel->interrupt->Halt();
			return;
		case SC_ReadNum:
		{
			ExceptionHandlerReadNum();
			break;
		}

		case SC_PrintNum:
		{
			ExceptionHandlerPrintNum();
			break;
		}

		case SC_RandomNum:
		{
			ExceptionHandlerRandomNum();
			break;
		}

		case SC_ReadChar:
		{
			ExceptionHandlerReadChar();
			break;
		}

		case SC_PrintChar:
		{
			ExceptionHandlerPrintChar();
			break;
		}

		case SC_ReadString:
		{
			ExceptionHandlerReadString();
			break;
		}

		case SC_PrintString:
		{
			ExceptionHandlerPrintString();
			break;
		}

		case SC_CreateFile:
		{
			ExceptionHandlerCreateFile();
			break;
		}

		case SC_Open:
		{
			ExceptionHandlerOpen();
			break;
		}

		case SC_Close:
		{
			ExceptionHandlerClose();
			break;
		}

		case SC_Read:
		{
			ExceptionHandlerRead();
			break;
		}

		case SC_Write:
		{
			ExceptionHandlerWrite();
			break;
		}

		case SC_Exec:
		{
			ExceptionHandlerExec();
			break;
		}

		case SC_Join:
		{
			ExceptionHandlerJoin();
			break;
		}

		case SC_Exit:
		{
			ExceptionHandlerExit();
			break;
		}

		case SC_CreateSemaphore:
		{
			ExceptionHandlerCreateSemaphore();
			break;
		}

		case SC_Wait:
		{
			ExceptionHandlerWait();
			break;
		}

		case SC_Signal:
		{
			ExceptionHandlerSignal();
			break;
		}

		default:
			break;
		}
		// after each SyscallException, we need to increase PC to do next instruction
		IncreasePC();
	}
}
