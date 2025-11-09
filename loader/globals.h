#ifndef LOADER_GLOBALS_H
#define LOADER_GLOBALS_H

#define _GNU_SOURCE
#include <stdio.h>
#include <elf.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <signal.h>
#include <ucontext.h>

// To display exceptions and exiting at runtime defined macros  are to be used along
// if statements or assertions to make it easer to log exceptions and exit at fatal 
// flaw. some extra MACROS for safe free calls for pointers
#define ERROR(message, x) { printf("%s\n", message);  }     // to display error message.
#define FATAL(message, x) { ERROR(message, x); exit(x); }   // to display & exit fatal.
#define CFARF(message, f) { close(f); FATAL(message,-1);}   // to close file and raise.
#define SFREE(pointer) if(pointer){ free(pointer); }        // to clean allocated memory.

#ifndef XINT32
#define XINT32
    #define uInt32 uint32_t
    #define sInt32  int32_t
#endif

// To store information about executable and make it accessible to all components
extern Elf32_Ehdr* ehdr;                    // stores ELF-Header of i386 EXECUTABLE
extern Elf32_Phdr* phdr;                    // stores PROG_Header (all) of i386 EXECUTABLE
extern sInt32        fd;                    // stores file descriptor of opened EXECUTABLE

extern volatile uInt32 page_fault_count;    // number of page FAULT OCCURED.
extern volatile uInt32 page_alloc_count;    // number of page ALLOCATED.

extern const uInt32 virtual_page_alignment_mask;
extern const uInt32 virtual_offset_address_mask;

#endif
