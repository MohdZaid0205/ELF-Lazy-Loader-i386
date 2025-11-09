#ifndef LOADER_SEGHANDLER_K
#define LOADER_SEGHANDLER_H

#include "globals.h"
#include "allocator.h"

void segmentation_fault_handler(int sig, siginfo_t* si, void* uctx);

#endif
