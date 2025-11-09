#ifndef LOADER_LOADER_H
#define LOADER_LOADER_H

#include "globals.h"
#include "seghandler.h"
#include "allocator.h"

void load_and_run_elf(char* exe);
void loader_cleanup();

#endif
