// addrspace.cc 
//	Routines to manage address spaces (executing user programs).
//
//	In order to run a user program, you must:
//
//	1. link with the -n -T 0 option 
//	2. run coff2noff to convert the object file to Nachos format
//		(Nachos object code format is essentially just a simpler
//		version of the UNIX executable object code format)
//	3. load the NOFF file into the Nachos file system
//		(if you are using the "stub" file system, you
//		don't need to do this last step)
//
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "main.h"
#include "addrspace.h"
#include "machine.h"
#ifndef NOFF_H
#define NOFF_H

#include "noff.h"

#endif
#include "bitmap.h"
#include "synch.h"

//----------------------------------------------------------------------
// SwapHeader
// 	Do little endian to big endian conversion on the bytes in the 
//	object file header, in case the file was generated on a little
//	endian machine, and we're now running on a big endian machine.
//----------------------------------------------------------------------

static void 
SwapHeader (NoffHeader *noffH)
{
    noffH->noffMagic = WordToHost(noffH->noffMagic);
    noffH->code.size = WordToHost(noffH->code.size);
    noffH->code.virtualAddr = WordToHost(noffH->code.virtualAddr);
    noffH->code.inFileAddr = WordToHost(noffH->code.inFileAddr);
#ifdef RDATA
    noffH->readonlyData.size = WordToHost(noffH->readonlyData.size);
    noffH->readonlyData.virtualAddr = 
           WordToHost(noffH->readonlyData.virtualAddr);
    noffH->readonlyData.inFileAddr = 
           WordToHost(noffH->readonlyData.inFileAddr);
#endif 
    noffH->initData.size = WordToHost(noffH->initData.size);
    noffH->initData.virtualAddr = WordToHost(noffH->initData.virtualAddr);
    noffH->initData.inFileAddr = WordToHost(noffH->initData.inFileAddr);
    noffH->uninitData.size = WordToHost(noffH->uninitData.size);
    noffH->uninitData.virtualAddr = WordToHost(noffH->uninitData.virtualAddr);
    noffH->uninitData.inFileAddr = WordToHost(noffH->uninitData.inFileAddr);

#ifdef RDATA
    DEBUG(dbgAddr, "code = " << noffH->code.size <<  
                   " readonly = " << noffH->readonlyData.size <<
                   " init = " << noffH->initData.size <<
                   " uninit = " << noffH->uninitData.size << "\n");
#endif
}

//----------------------------------------------------------------------
// AddrSpace::AddrSpace
// 	Create an address space to run a user program.
//	Set up the translation from program memory to physical 
//	memory.  For now, this is really simple (1:1), since we are
//	only uniprogramming, and we have a single unsegmented page table
//----------------------------------------------------------------------

AddrSpace::AddrSpace()
{
    DEBUG(dbgAddr, "Getting in AddrSpace::AddrSpace()\n");
    this->pageTable = NULL;
    this->numPages = 0;
}

//----------------------------------------------------------------------
// AddrSpace::~AddrSpace
// 	Dealloate an address space.
//----------------------------------------------------------------------
AddrSpace::~AddrSpace()
{
    int i;
    for (i = 0; i < numPages; ++i) {
        kernel->gPhysPageBitMap->Clear(pageTable[i].physicalPage);
    }
    delete pageTable;
}

bool AddrSpace::Load(char* fileName)
{
    DEBUG(dbgAddr, "Getting in AddrSpace::Load(\"" << fileName << "\")");

    // ---- Open executable file and read information from the header ----

    OpenFile* executable = kernel->fileSystem->Open(fileName);
    if (executable == NULL) {
        fprintf(stderr, "Addrspace: Error opening file: %s", fileName);
        return FALSE;
    }

    // Read NOFF header
    NoffHeader noffH;
    executable->ReadAt((char*) &noffH, sizeof(noffH), 0);
    if ((noffH.noffMagic != NOFFMAGIC) &&
        (WordToHost(noffH.noffMagic) == NOFFMAGIC)) {
        SwapHeader(&noffH);
    }
    ASSERT(noffH.noffMagic == NOFFMAGIC);
    
    
    // Size of the address space in bytes
    int addressSpaceSize;
    addressSpaceSize =
        noffH.code.size + noffH.readonlyData.size +
        noffH.initData.size + noffH.uninitData.size + UserStackSize;
    DEBUG(
        dbgAddr,
        "Read " << fileName << " with total program size: " << addressSpaceSize
    );
    DEBUG(
        dbgAddr,
        "Code segment: " << noffH.code.size <<
        ", initData segment: " << noffH.initData.size <<
        ", uninitData segment: " << noffH.uninitData.size <<
        ", stack: " << UserStackSize
    )
    
    // Determine the number of virtual pages required
    this->numPages = divRoundUp(addressSpaceSize, PageSize);
    addressSpaceSize = this->numPages * PageSize;
    DEBUG(dbgAddr, "Allocating " << this->numPages << " pages ...");
    
    // ---- Allocate physical frames ---

    kernel->addrLock->P();
    char *mem = kernel->machine->mainMemory;

    if (this->numPages > kernel->gPhysPageBitMap->NumClear()) {
        fprintf(stderr, "AddrSpace: Not enough memory for new process..!\n");
        this->numPages = 0;
        delete executable;
        kernel->addrLock->V();
        return FALSE;
    }

    // Allocate free frames
    DEBUG(dbgAddr, "Creating new page table...");
    this->pageTable = new TranslationEntry[numPages];
    for (int i = 0; i < numPages; i++) {
        this->pageTable[i].virtualPage = i;
        int clearFrame = kernel->gPhysPageBitMap->FindAndSet();
        this->pageTable[i].physicalPage = clearFrame;
        DEBUG(
            dbgAddr,
            "Assigned virtual page " << this->pageTable[i].virtualPage <<
            " to frame " << this->pageTable[i].physicalPage
        );
        ASSERT(clearFrame != -1);

        this->pageTable[i].valid = TRUE;
        this->pageTable[i].use = FALSE;
        this->pageTable[i].dirty = FALSE;
        this->pageTable[i].readOnly = FALSE;
        
        // Zero out these frames
        bzero(
            mem + this->pageTable[i].physicalPage * PageSize,
            PageSize
        );
    }


    // ---- Load program code and data into memory ----

    // The actual size of the segments required to be loaded, the uninitData
    // and stack is left empty.
    int actualSize =
        noffH.code.size + noffH.initData.size + noffH.readonlyData.size;

    // Load all the code, initData and readonlyData segments into buffer
    char *buffer = new char[actualSize];
    for (int i = 0; i < actualSize; ++i) {
        buffer[i] = NULL;
    }
    int numSegments = 3;
    Segment *seg[] = {&noffH.code, &noffH.readonlyData, &noffH.initData};
    for (int i = 0; i < numSegments; ++i) {
        // The virtual address of the segment in the NOFF file is the same as
        // the virtual address of the starting position of the segment in the
        // virtual address space
        int vaddr = seg[i]->virtualAddr;
        int size = seg[i]->size;
        if (size != 0) {
            executable->ReadAt(
                buffer + vaddr,
                seg[i]->size,
                seg[i]->inFileAddr
            );
        }
    }

    // With the buffer now contains the data that is already in place, copy
    // the buffer into appropriate location in the physical memory

    // Number of pages for the require-loading data
    int pages = divRoundUp(actualSize, PageSize);
    int lastPage = actualSize % PageSize;
    if (lastPage == 0) {
        lastPage = PageSize;
    }
    for (int i = 0; i < pages; ++i) {
        memcpy(
            mem + pageTable[i].physicalPage * PageSize,
            buffer + i * PageSize,
            (i == pages - 1) ? lastPage : PageSize
        );
    }

    kernel->addrLock->V();
    delete[] buffer;
    delete executable;
    return TRUE;
}

//----------------------------------------------------------------------
// AddrSpace::Execute
// 	Run a user program using the current thread
//
//      The program is assumed to have already been loaded into
//      the address space
//
//----------------------------------------------------------------------

void 
AddrSpace::Execute() 
{

    kernel->currentThread->space = this;

    this->InitRegisters();		// set the initial register values
    this->RestoreState();		// load page table register

    kernel->machine->Run();		// jump to the user progam

    ASSERTNOTREACHED();			// machine->Run never returns;
					// the address space exits
					// by doing the syscall "exit"
}


//----------------------------------------------------------------------
// AddrSpace::InitRegisters
// 	Set the initial values for the user-level register set.
//
// 	We write these directly into the "machine" registers, so
//	that we can immediately jump to user code.  Note that these
//	will be saved/restored into the currentThread->userRegisters
//	when this thread is context switched out.
//----------------------------------------------------------------------

void
AddrSpace::InitRegisters()
{
    Machine *machine = kernel->machine;
    int i;

    for (i = 0; i < NumTotalRegs; i++)
	machine->WriteRegister(i, 0);

    // Initial program counter -- must be location of "Start", which
    //  is assumed to be virtual address zero
    machine->WriteRegister(PCReg, 0);	

    // Need to also tell MIPS where next instruction is, because
    // of branch delay possibility
    // Since instructions occupy four bytes each, the next instruction
    // after start will be at virtual address four.
    machine->WriteRegister(NextPCReg, 4);

   // Set the stack register to the end of the address space, where we
   // allocated the stack; but subtract off a bit, to make sure we don't
   // accidentally reference off the end!
    machine->WriteRegister(StackReg, numPages * PageSize - 16);
    DEBUG(dbgAddr, "Initializing stack pointer: " << numPages * PageSize - 16);
}

//----------------------------------------------------------------------
// AddrSpace::SaveState
// 	On a context switch, save any machine state, specific
//	to this address space, that needs saving.
//
//	For now, don't need to save anything!
//----------------------------------------------------------------------

void AddrSpace::SaveState() 
{}

//----------------------------------------------------------------------
// AddrSpace::RestoreState
// 	On a context switch, restore the machine state so that
//	this address space can run.
//
//      For now, tell the machine where to find the page table.
//----------------------------------------------------------------------

void AddrSpace::RestoreState() 
{
    kernel->machine->pageTable = pageTable;
    kernel->machine->pageTableSize = numPages;
}


//----------------------------------------------------------------------
// AddrSpace::Translate
//  Translate the virtual address in _vaddr_ to a physical address
//  and store the physical address in _paddr_.
//  The flag _isReadWrite_ is false (0) for read-only access; true (1)
//  for read-write access.
//  Return any exceptions caused by the address translation.
//----------------------------------------------------------------------
ExceptionType
AddrSpace::Translate(unsigned int vaddr, unsigned int *paddr, int isReadWrite)
{
    TranslationEntry *pte;
    int               pfn;
    unsigned int      vpn    = vaddr / PageSize;
    unsigned int      offset = vaddr % PageSize;

    if(vpn >= numPages) {
        return AddressErrorException;
    }

    pte = &pageTable[vpn];

    if(isReadWrite && pte->readOnly) {
        return ReadOnlyException;
    }

    pfn = pte->physicalPage;

    // if the pageFrame is too big, there is something really wrong!
    // An invalid translation was loaded into the page table or TLB.
    if (pfn >= NumPhysPages) {
        DEBUG(dbgAddr, "Illegal physical page " << pfn);
        return BusErrorException;
    }

    pte->use = TRUE;          // set the use, dirty bits

    if(isReadWrite)
        pte->dirty = TRUE;

    *paddr = pfn*PageSize + offset;

    ASSERT((*paddr < MemorySize));

    //cerr << " -- AddrSpace::Translate(): vaddr: " << vaddr <<
    //  ", paddr: " << *paddr << "\n";

    return NoException;
}