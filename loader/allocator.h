#ifndef LOADER_ALLOCATOR_H
#define LOADER_ALLOCATOR_H

#include "globals.h"

uInt32 align_page_address(uInt32 vaddr);
uInt32 align_offs_address(uInt32 vaddr);

uInt32 allocate_page(uInt32 vaddr);

#endif
