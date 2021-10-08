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

// Doi thanh ghi Program counter cua he thong ve sau 4 byte de tiep tuc nap lenh
void IncreasePC()
{
	int counter = kernel->machine->ReadRegister(PCReg);
	kernel->machine->WriteRegister(PrevPCReg, counter);
	counter = kernel->machine->ReadRegister(NextPCReg);
	kernel->machine->WriteRegister(PCReg, counter);
	kernel->machine->WriteRegister(NextPCReg, counter + 4);
}

// Input: Khong gian dia chi User(int) - gioi han cua buffer(int)
// Output: Bo nho dem Buffer(char*)
// Chuc nang: Sao chep vung nho User sang vung nho System
char *User2System(int virtAddr, int limit)
{
	int i; //chi so index
	int oneChar;
	char *kernelBuf = NULL;
	kernelBuf = new char[limit + 1]; //can cho chuoi terminal
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

// Input: Khong gian vung nho User(int) - gioi han cua buffer(int) - bo nho dem buffer(char*)
// Output: So byte da sao chep(int)
// Chuc nang: Sao chep vung nho System sang vung nho User
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

void ExceptionHandlerReadNum()
{
	// Input: None
	// Output: Return an Integer from console
	// Usage: Read an Integer from console

	// int: [-2147483648 , 2147483647] --> max length = 10 not count char '-'
	// Handle to not overflow int by not to allow system read more than length of int
	const int MAX_BUFFER = 255;
	char *num_buffer = new char[MAX_BUFFER + 1];
	bool isPositive = true;
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
			printf("\n\nThe integer number is not valid");
			DEBUG('a', "\nThe integer number is not valid");
			kernel->machine->WriteRegister(2, 0);
			IncreasePC();
			delete num_buffer;
			return;
		}
	}

	int length_num = i; // length of number_buffer use for number

	// if input is only '-' then we throw error
	if (!isPositive && length_num == 0)
	{
		printf("\n\nThe integer number is not valid");
		DEBUG('a', "\nThe integer number is not valid");
		kernel->machine->WriteRegister(2, 0);
		IncreasePC();
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
		IncreasePC();
		delete num_buffer;
		return;
	}

	// if res is negative, then opposite ret; else keep the result
	res = (!isPositive) ? (-res) : res;

	// write result res into kernel
	kernel->machine->WriteRegister(2, (int)res);
}

void ExceptionHandlerPrintNum()
{
	// Input: an Integer
	// Output: None
	// Usage: print an Integer on console

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

void ExceptionHandlerRandomNum()
{
	// Input: None
	// Output: return an integer
	// Usage: create and return a random number

	srand(time(NULL));

	// random positive number
	int number;
	do
	{
		// int: [-2147483648 , 2147483647] --> rand from 1 to 2147483647
		number = rand() % 2147483647;
	} while (number == 0);

	// write number to kernel
	kernel->machine->WriteRegister(2, number);
}

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

void ExceptionHandlerPrintString()
{
	// Retrieve the string address in user space
	int addr = kernel->machine->ReadRegister(4);

	// Copy the string into kernel space
	char *buffer;
	buffer = User2System(addr, 255);

	// Find the string's length
	int len = 0;
	while (buffer[len] != '\0' and len < 255)
		len++;

	// Print the string to console
	SysPrintString(buffer, len);
	delete buffer;
}

void ExceptionHandlerReadChar()
{
	char c = kernel->synchConsoleIn->GetChar();
	kernel->machine->WriteRegister(2, c);
}

void ExceptionHandlerPrintChar()
{
	char c = (char)kernel->machine->ReadRegister(4);
	kernel->synchConsoleOut->PutChar(c);
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

		default:
			break;
		}
		// after each SyscallException, we need to increase PC to do next instruction
		IncreasePC();
	}
}
