// main.h 
//	This file defines the Nachos global variables
//
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#ifndef MAIN_H
#define MAIN_H

#include "bitmap.h"
#include "copyright.h"
#include "debug.h"
#include "kernel.h"
#include "stable.h" 

extern Kernel *kernel;
extern Debug *debug;

class Bitmap;
class Semaphore;
class PTable;

// Bitmap data structure for managing physical memory pages
extern Bitmap *gPhysPageBitMap;
extern Semaphore *addrLock;

// Data structure for managing PCBs
extern PTable *pTab;

#endif // MAIN_H

