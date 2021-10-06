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
			// Input: K co
			// Output: Tra ve so nguyen doc duoc tu man hinh console.
			// Chuc nang: Doc so nguyen tu man hinh console.
			char *buffer;
			int MAX_BUFFER = 255;
			buffer = new char[MAX_BUFFER + 1];
			int numbytes = kernel->synchConsoleIn->GetChar(); //->Read(buffer, MAX_BUFFER); // doc buffer toi da MAX_BUFFER ki tu, tra ve so ki tu doc dc
			int number = 0;											// so luu ket qua tra ve cuoi cung

			/* Qua trinh chuyen doi tu buffer sang so nguyen int */

			// Xac dinh so am hay so duong
			bool isNegative = false; // Gia thiet la so duong.
			int firstNumIndex = 0;
			int lastNumIndex = 0;
			if (buffer[0] == '-')
			{
				isNegative = true;
				firstNumIndex = 1;
				lastNumIndex = 1;
			}

			// Kiem tra tinh hop le cua so nguyen buffer
			for (int i = firstNumIndex; i < numbytes; i++)
			{
				if (buffer[i] == '.') /// 125.0000000 van la so
				{
					int j = i + 1;
					for (; j < numbytes; j++)
					{
						// So khong hop le
						if (buffer[j] != '0')
						{
							printf("\n\n The integer number is not valid");
							DEBUG('a', "\n The integer number is not valid");
							kernel->machine->WriteRegister(2, 0);
							IncreasePC();
							delete buffer;
							return;
						}
					}
					// la so thoa cap nhat lastNumIndex
					lastNumIndex = i - 1;
					break;
				}
				else if (buffer[i] < '0' && buffer[i] > '9')
				{
					printf("\n\n The integer number is not valid");
					DEBUG('a', "\n The integer number is not valid");
					kernel->machine->WriteRegister(2, 0);
					IncreasePC();
					delete buffer;
					return;
				}
				lastNumIndex = i;
			}

			// La so nguyen hop le, tien hanh chuyen chuoi ve so nguyen
			for (int i = firstNumIndex; i <= lastNumIndex; i++)
			{
				number = number * 10 + (int)(buffer[i] - 48);
			}

			// neu la so am thi * -1;
			if (isNegative)
			{
				number = number * -1;
			}
			kernel->machine->WriteRegister(2, number);
			IncreasePC();
			delete buffer;
			return;

		case SC_PrintNum:
			// Input: mot so integer
			// Output: khong co
			// Chuc nang: In so nguyen len man hinh console
			int number = kernel->machine->ReadRegister(4);
			if (number == 0)
			{
				gSynchConsole->Write("0", 1); // In ra man hinh so 0
				IncreasePC();
				return;
			}

			/*Qua trinh chuyen so thanh chuoi de in ra man hinh*/
			bool isNegative = false; // gia su la so duong
			int numberOfNum = 0;	 // Bien de luu so chu so cua number
			int firstNumIndex = 0;

			if (number < 0)
			{
				isNegative = true;
				number = number * -1; // Nham chuyen so am thanh so duong de tinh so chu so
				firstNumIndex = 1;
			}

			int t_number = number; // bien tam cho number
			while (t_number)
			{
				numberOfNum++;
				t_number /= 10;
			}

			// Tao buffer chuoi de in ra man hinh
			char *buffer;
			int MAX_BUFFER = 255;
			buffer = new char[MAX_BUFFER + 1];
			for (int i = firstNumIndex + numberOfNum - 1; i >= firstNumIndex; i--)
			{
				buffer[i] = (char)((number % 10) + 48);
				number /= 10;
			}
			if (isNegative)
			{
				buffer[0] = '-';
				buffer[numberOfNum + 1] = 0;
				gSynchConsole->Write(buffer, numberOfNum + 1);
				delete buffer;
				IncreasePC();
				return;
			}
			buffer[numberOfNum] = 0;
			gSynchConsole->Write(buffer, numberOfNum);
			delete buffer;
			IncreasePC();
			return;

		case SC_ReadChar:
			//Input: Khong co
			//Output: Duy nhat 1 ky tu (char)
			//Cong dung: Doc mot ky tu tu nguoi dung nhap
			char c = kernel->synchConsoleIn->GetChar();
			kernel->machine->WriteRegister(2, c);
			//IncreasePC(); // error system
			//return;
			break;

		case SC_PrintChar:
			// Input: Ki tu(char)
			// Output: Ki tu(char)
			// Cong dung: Xuat mot ki tu la tham so arg ra man hinh
			char c = (char)kernel->machine->ReadRegister(4); // Doc ki tu tu thanh ghi r4
			gSynchConsole->Write(&c, 1);					 // In ky tu tu bien c, 1 byte
			//IncreasePC();
			break;

		case SC_ReadString:
			// Input: Buffer(char*), do dai toi da cua chuoi nhap vao(int)
			// Output: Khong co
			// Cong dung: Doc vao mot chuoi voi tham so la buffer va do dai toi da
			int virtAddr, length;
			char *buffer;
			virtAddr = kernel->machine->ReadRegister(4); // Lay dia chi tham so buffer truyen vao tu thanh ghi so 4
			length = kernel->machine->ReadRegister(5);	 // Lay do dai toi da cua chuoi nhap vao tu thanh ghi so 5
			buffer = User2System(virtAddr, length);		 // Copy chuoi tu vung nho User Space sang System Space
			gSynchConsole->Read(buffer, length);		 // Goi ham Read cua SynchConsole de doc chuoi
			System2User(virtAddr, length, buffer);		 // Copy chuoi tu vung nho System Space sang vung nho User Space
			delete buffer;
			IncreasePC(); // Tang Program Counter
			return;
			//break;

		case SC_PrintString:
			// Input: Buffer(char*)
			// Output: Chuoi doc duoc tu buffer(char*)
			// Cong dung: Xuat mot chuoi la tham so buffer truyen vao ra man hinh
			int virtAddr;
			char *buffer;
			virtAddr = kernel->machine->ReadRegister(4); // Lay dia chi cua tham so buffer tu thanh ghi so 4
			buffer = User2System(virtAddr, 255);		 // Copy chuoi tu vung nho User Space sang System Space voi bo dem buffer dai 255 ki tu
			int length = 0;
			while (buffer[length] != 0)
				length++;							  // Dem do dai that cua chuoi
			gSynchConsole->Write(buffer, length + 1); // Goi ham Write cua SynchConsole de in chuoi
			delete buffer;
			//IncreasePC(); // Tang Program Counter
			//return;
			break;

		default:
			break;
		}
		IncreasePC();
	}
}
