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
		DEBUG(dbgMach, "The integer number is not valid");
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
	// int CreateFile(char* filename)
	int virtAddr;
	char *filename;

	// read filename from register r4
	virtAddr = kernel->machine->ReadRegister(4);
	// copy from user space to kernel space
	filename = User2System(virtAddr, MaxFileLength + 1);

	// filename len = 0
	if (strlen(filename) == 0)
	{
		printf("\nFile name is not valid");
		DEBUG(dbgFile, "\nFile name is not valid");
		kernel->machine->WriteRegister(2, -1); // fail
		delete[] filename;
		return;
	}

	// can't read filename
	if (filename == NULL)
	{
		printf("\nNot enough memory in system");
		DEBUG(dbgFile, "\nNot enough memory in system");
		kernel->machine->WriteRegister(2, -1); //  fail
		delete[] filename;
		return;
	}
	// DEBUG('a', "\nFinish reading filename.");

	// fail in create file
	if (!kernel->fileSystem->Create(filename, 0))
	{
		printf("\nError create file '%s'", filename);
		DEBUG(dbgFile, "\nError create file '%s'" << filename);
		kernel->machine->WriteRegister(2, -1); // fail
		delete[] filename;
		return;
	}

	kernel->machine->WriteRegister(2, 0); // success
	delete[] filename;
}

// Usage: Open a file
// Input: 	arg1: name address,
//			arg2: type -> 0: read and write, 1: read only, 2: stdin, 3: stdout
// Output: return OpenFileId, if fail then return -1
void ExceptionHandlerOpen()
{
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
		printf("\nFull slot in openTable");
		DEBUG(dbgFile, "\nFull slot in openTable");
		kernel->machine->WriteRegister(2, -1); // write -1 to register r2
		delete[] filename;
		return;
	}

	// Check each type of open file
	switch (type)
	{
	case 0: //  Read and write
	case 1: //  Read only
		if ((kernel->fileSystem->openTable[freeSlot] = kernel->fileSystem->Open(filename, type)))
		{
			kernel->machine->WriteRegister(2, freeSlot); // success -> write OpenFileID to register r2
		}
		else
		{
			printf("\nFile does not exist");
			DEBUG(dbgFile, "\nFile does not exist");
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
		printf("\nType is not match");
		DEBUG(dbgFile, "\nType is not match");
		kernel->machine->WriteRegister(2, -1); // fail
	}
	delete[] filename;
}

// Usage: Close a file
// Input :  id of file (OpenFileId)
// Output : success: 0, fail: -1
void ExceptionHandlerClose()
{
	// int Close(OpenFileID id)
	int fileID;

	fileID = kernel->machine->ReadRegister(4); // read fileID from register r4
	if (fileID >= 0 && fileID < SIZE_TABLE)
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
			printf("\nFile does not exist");
			DEBUG(dbgFile, "\nFile does not exist");
			kernel->machine->WriteRegister(2, -1); // fail
			return;
		}
	}
	printf("\nFileID is not match");
	DEBUG(dbgFile, "\nFile is not match");
	kernel->machine->WriteRegister(2, -1); // fail
}

// Usage: Read from file
// Input :  buffer address, number of bytes to read, fileID
// Output : success: number of bytes read, fail: -1, EOF: -2
void ExceptionHandlerRead()
{
	// int Read(char *buffer, int size, OpenFileId id);
	int virtAddr;
	int size;
	int fileID;
	char *buffer;

	virtAddr = kernel->machine->ReadRegister(4); // read buffer address from register r4
	size = kernel->machine->ReadRegister(5);	 // read size from register r5
	fileID = kernel->machine->ReadRegister(6);	 // read fileID from register r6

	// fileID is not match
	if (fileID < 0 || fileID >= SIZE_TABLE)
	{
		printf("FileID is not match\n");
		DEBUG(dbgFile, "FileID is not match");
		kernel->machine->WriteRegister(2, -1); // fail
		return;
	}

	// fileID does not exist
	if (kernel->fileSystem->openTable[fileID] == NULL)
	{
		printf("FileID does not exist\n");
		DEBUG(dbgFile, "FileID does not exist");
		kernel->machine->WriteRegister(2, -1); // fail
		return;
	}

	// read from stdout
	if (kernel->fileSystem->openTable[fileID]->type == 3)
	{
		printf("Cannot read from stdout\n");
		DEBUG(dbgFile, "Cannot read from stdout");
		kernel->machine->WriteRegister(2, -1); // fail
		return;
	}

	buffer = User2System(virtAddr, size); // Copy buffer form user space to system space

	// read from stdin
	if (kernel->fileSystem->openTable[fileID]->type == 2)
	{
		int count = kernel->synchConsoleIn->Read(buffer, size); // Transfer data to buffer from console
		System2User(virtAddr, count, buffer);					// Copy data buffer to user space
		kernel->machine->WriteRegister(2, count);				// success -> write count to register r2
	}

	// read from file
	// file only read or read and write
	else if (kernel->fileSystem->openTable[fileID]->type == 0 || kernel->fileSystem->openTable[fileID]->type == 1)
	{
		int count = kernel->fileSystem->openTable[fileID]->Read(buffer, size);
		if (count > 0)
		{
			System2User(virtAddr, count, buffer);	  // Copy data buffer to user space
			kernel->machine->WriteRegister(2, count); // success -> write count to register r2
		}
		else // EOF
		{
			// printf("\nEOF");
			// DEBUG('a', "\nEOF");
			kernel->machine->WriteRegister(2, -2);
		}
	}

	delete[] buffer;
}

// Usage: Write to file
// Input :  buffer address, number of bytes to write, fileID
// Output : success: number of bytes written, fail: -1
void ExceptionHandlerWrite()
{
	// int Write(char *buffer, int size, OpenFileId id)
	int virtAddr;
	int size;
	int fileID;
	char *buffer;

	virtAddr = kernel->machine->ReadRegister(4); // read buffer address from register r4
	size = kernel->machine->ReadRegister(5);	 // read size from register r5
	fileID = kernel->machine->ReadRegister(6);	 // read fileID from register r6

	// fileId is not match
	if (fileID < 0 || fileID >= SIZE_TABLE)
	{
		// printf("FileID is not match\n");
		DEBUG(dbgFile, "FileID is not match");
		kernel->machine->WriteRegister(2, -1);
		return;
	}

	// fileId does not exist
	if (kernel->fileSystem->openTable[fileID] == NULL)
	{
		// printf("FileID does not exist\n");
		DEBUG(dbgFile, "FileID does not exist");
		kernel->machine->WriteRegister(2, -1);
		return;
	}

	// write to read-only file
	if (kernel->fileSystem->openTable[fileID]->type == 1)
	{
		// printf("Cannot write to read-only file");
		DEBUG(dbgFile, "Cannot write to read-only file");
		kernel->machine->WriteRegister(2, -1);
		return;
	}

	// write to stdin
	if (kernel->fileSystem->openTable[fileID]->type == 2)
	{
		// printf("Cannot write to stdin\n");
		DEBUG(dbgFile, "Cannot write to stdin");
		kernel->machine->WriteRegister(2, -1);
		return;
	}

	buffer = User2System(virtAddr, size); // Copy buffer form user space to system space

	// write to stdout
	if (kernel->fileSystem->openTable[fileID]->type == 3)
	{
		int count = 0;
		while (buffer[count] != '\0' && count < size) // loop until meet '\0' or out of size
			count++;

		count = kernel->synchConsoleOut->Write(buffer, count); // write data
		buffer[count] = '\n';
		kernel->synchConsoleOut->Write(buffer + count, 1); // write '\n'
		kernel->machine->WriteRegister(2, count - 1);	   // return real bytes written
	}

	// write to file
	else if (kernel->fileSystem->openTable[fileID]->type == 0)
	{
		int count = kernel->fileSystem->openTable[fileID]->Write(buffer, size);

		if (count > 0)
		{
			kernel->machine->WriteRegister(2, count); // success -> write count to register r2
		}
		else
		{
			// printf("Write fail\n");
			DEBUG(dbgFile, "Write fail");
			kernel->machine->WriteRegister(2, -1);
		}
	}

	delete[] buffer;
}

// Usage: Seek in file
// Input : position of cursor, fileID
// Output : success: real position of cursor, fail: -1
void ExceptionHandlerSeek()
{
	// int Seek(int pos, OpenFileId id)
	int position;
	int fileID;

	position = kernel->machine->ReadRegister(4); // read position from register r4
	fileID = kernel->machine->ReadRegister(5);	 // read fileID from register r5

	// fileId is not match
	if (fileID < 0 || fileID >= SIZE_TABLE)
	{
		// printf("FileID is not match\n");
		DEBUG(dbgFile, "FileID is not match");
		kernel->machine->WriteRegister(2, -1);
		return;
	}

	// fileId does not exist
	if (kernel->fileSystem->openTable[fileID] == NULL)
	{
		// printf("FileID does not exist\n");
		DEBUG(dbgFile, "FileID does not exist");
		kernel->machine->WriteRegister(2, -1);
		return;
	}

	// seek to stdin or stdout
	if (kernel->fileSystem->openTable[fileID]->type == 2 || kernel->fileSystem->openTable[fileID]->type == 3)
	{
		// printf("Cannot seek to stdin or stdout\n");
		DEBUG(dbgFile, "Cannot seek to stdin or stdout");
		kernel->machine->WriteRegister(2, -1);
		return;
	}

	// if position = -1, seek cursor to end of file
	// DEBUG(dbgFile,position);
	if (position == -1)
	{
		position = kernel->fileSystem->openTable[fileID]->Length();
		// DEBUG(dbgFile,"SIZE :" << position);
	}

	// position is not match
	if (position < 0 || position > kernel->fileSystem->openTable[fileID]->Length())
	{
		// printf("Position is not match\n");
		DEBUG(dbgFile, "Position is not match");
		kernel->machine->WriteRegister(2, -1);
		return;
	}

	// Seek cursor to position
	kernel->fileSystem->openTable[fileID]->Seek(position);
	// DEBUG(dbgFile,position);
	kernel->machine->WriteRegister(2, position);
}

// Usage: create a process from a program and schedule it for execution
// Input: address to the program name
// Output: the process ID, or -1 on failure
void ExceptionHandlerExec()
{
	DEBUG(dbgSys, "Syscall: Exec(filename)");

	int addr = kernel->machine->ReadRegister(4);
	DEBUG(dbgSys, "Register 4: " << addr);

	char *fileName;
	fileName = User2System(addr, 255);
	DEBUG(dbgSys, "Read file name: " << fileName);

	DEBUG(dbgSys, "Scheduling execution...");
	int result = kernel->pTab->ExecUpdate(fileName);

	DEBUG(dbgSys, "Writing result to register 2: " << result);
	kernel->machine->WriteRegister(2, result);
	delete fileName;
}

// Usage: block the current thread until the child thread has exited
// Input: ID of the thread being joined
// Output: exit code of the thread
void ExceptionHandlerJoin()
{
	DEBUG(dbgSys, "Syscall: Join");
	int id = kernel->machine->ReadRegister(4);
	int result = kernel->pTab->JoinUpdate(id);
	kernel->machine->WriteRegister(2, result);
}

// Usage: exit current thread
// Input: exit code to pass to parent
// Output: none
void ExceptionHandlerExit()
{
	DEBUG(dbgSys, "Syscall: Exit");
	int exitCode = kernel->machine->ReadRegister(4);
	int result = kernel->pTab->ExitUpdate(exitCode);
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
		// DEBUG(dbgSynch, "\nNot enough memory in System");
		printf("\nNot enough memory in System");
		kernel->machine->WriteRegister(2, -1);
		delete[] name;
		return;
	}
	
	int res = kernel->semTab->Create(name, semVal);

	// Check error
	if(res == -1)
	{
		// DEBUG('a', "\nCan not create semaphore");
		printf("\nCan not create semaphore");
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
		// DEBUG(dbgSynch, "\nNot enough memory in System");
		printf("\nNot enough memory in System");
		kernel->machine->WriteRegister(2, -1);
		delete[] name;
		return;
	}

	int res = kernel->semTab->Wait(name);
	
	// Check error
	if(res == -1)
	{
		// DEBUG(dbgSynch, "\nNot exists semaphore");
		printf("\nNot exists semaphore");
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
		// DEBUG(dbgSynch, "\nNot enough memory in System");
		printf("\n Not enough memory in System");
		kernel->machine->WriteRegister(2, -1);
		delete[] name;
		return;
	}
	
	int res = kernel->semTab->Signal(name);

	// Check error
	if(res == -1)
	{
		// DEBUG(dbgSynch, "\nNot exists semaphore");
		printf("\nNot exists semaphore");
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
		DEBUG(dbgSys, "Unexpected user mode exception PageFaultException");
		kernel->interrupt->Halt();
		break;

	case ReadOnlyException:
		DEBUG(dbgSys, "\nUnexpected user mode exception ReadOnlyException");
		kernel->interrupt->Halt();
		break;

	case BusErrorException:
		DEBUG(dbgSys, "\nUnexpected user mode exception BusErrorException");
		kernel->interrupt->Halt();
		break;

	case AddressErrorException:
		DEBUG(dbgSys, "\nUnexpected user mode exception AddressErrorException");
		kernel->interrupt->Halt();
		break;

	case OverflowException:
		DEBUG(dbgSys, "\nUnexpected user mode exception OverflowException");
		kernel->interrupt->Halt();
		break;

	case IllegalInstrException:
		DEBUG(dbgSys, "\nUnexpected user mode exception IllegalInstrException");
		kernel->interrupt->Halt();
		break;

	case NumExceptionTypes:
		DEBUG(dbgSys, "\nUnexpected user mode exception NumExceptionTypes");
		kernel->interrupt->Halt();
		break;

	case SyscallException:
		switch (type)
		{
		case SC_Halt:
			// Input: None
			// Output: Shutdown notification
			// Usage: Shutdown

			DEBUG(dbgSys, "Shutdown, initiated by user program. ");
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

		case SC_Seek:
		{
			ExceptionHandlerSeek();
			break;
		}

		default:
			break;
		}
		// after each SyscallException, we need to increase PC to do next instruction
		IncreasePC();
	}
}
