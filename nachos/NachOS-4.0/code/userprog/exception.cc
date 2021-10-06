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

void ExceptionHandler(ExceptionType which)
{
	int type = kernel->machine->ReadRegister(2);

	switch (which)
	{
	case NoException:
		return;
	case PageFaultException:
		DEBUG('a', "\n No valid translation found");
		printf("\n\n No valid translation found");
		kernel->interrupt->Halt();
		break;

	case ReadOnlyException:
		DEBUG('a', "\n Write attempted to page marked read-only");
		printf("\n\n Write attempted to page marked read-only");
		kernel->interrupt->Halt();
		break;

	case BusErrorException:
		DEBUG('a', "\n Translation resulted invalid physical address");
		printf("\n\n Translation resulted invalid physical address");
		kernel->interrupt->Halt();
		break;

	case AddressErrorException:
		DEBUG('a', "\n Unaligned reference or one that was beyond the end of the address space");
		printf("\n\n Unaligned reference or one that was beyond the end of the address space");
		kernel->interrupt->Halt();
		break;

	case OverflowException:
		DEBUG('a', "\nInteger overflow in add or sub.");
		printf("\n\n Integer overflow in add or sub.");
		kernel->interrupt->Halt();
		break;

	case IllegalInstrException:
		DEBUG('a', "\n Unimplemented or reserved instr.");
		printf("\n\n Unimplemented or reserved instr.");
		kernel->interrupt->Halt();
		break;

	case NumExceptionTypes:
		DEBUG('a', "\n Number exception types");
		printf("\n\n Number exception types");
		kernel->interrupt->Halt();
		break;

	case SyscallException:
		switch (type)
		{
		case SC_Halt:
			// Input: Khong co
			// Output: Thong bao tat may
			// Chuc nang: Tat HDH
			DEBUG('a', "\nShutdown, initiated by user program. ");
			printf("\nShutdown, initiated by user program. ");
			kernel->interrupt->Halt();
			return;
		case SC_ReadNum:
			break;
		case SC_PrintNum:
			break;
		case SC_ReadChar:
			//Input: Khong co
			//Output: Duy nhat 1 ky tu (char)
			//Cong dung: Doc mot ky tu tu nguoi dung nhap
			char c = kernel->synchConsoleIn->GetChar();
			kernel->machine->WriteRegister(2, c);
			break;

		case SC_PrintChar:
			// Input: Ki tu(char)
			// Output: Ki tu(char)
			// Cong dung: Xuat mot ki tu la tham so arg ra man hinh
			char c = (char)kernel->machine->ReadRegister(4); // Doc ki tu tu thanh ghi r4
			kernel->synchConsoleOut->PutChar(c);					 // In ky tu tu bien c, 1 byte
			//IncreasePC();
			break;

		case SC_ReadString:
		case SC_PrintString:
		default:
			break;
		}
		IncreasePC();
	}
}
