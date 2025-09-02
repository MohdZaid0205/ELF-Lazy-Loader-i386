#include "loader.h"

// important headers and file to be loaded, executed and cleaned in corresponding functions
Elf32_Ehdr* ehdr;   // stores elf header, see [>_] for informations regarding readings.  
Elf32_Phdr* phdr;   // stores program header, see [>_] for informations on structure.
int         fd;     // stored file-descriptor, to open/close and read/seek when needed.
void*       vm;     // virtual map for protected memory and related information.

// To display exceptions and exiting at runtime defined macros  are to be used along
// if statements or assertions to make it easer to log exceptions and exit at fatal flaw.
#define ERROR(message, x) { printf("%s\n", message);  }     // to display error message.
#define FATAL(message, x) { ERROR(message, x); exit(x); }   // to display and exit fatal.
#define CFARF(message, f) { close(f); FATAL(message,-1);}   // to close file and raise.
#define SFREE(pointer) if(pointer){ free(pointer); }        // to clean allocated memory.

/** [>_] How this solution load elf-header directly from executable and why it works?
 *    -> assembler does not dump in memory content of Xhdr directly in executable/objects.
 *    -> but it is done in manner defined by elf-spec specifications, which on most platf
 *    -> matches to structure of ElfXX_Xhdr in itself. (so in - memory load works fine).
*/

void loader_cleanup(){
    int status = munmap(vm, phdr->p_memsz); // deallocate virtual map
    SFREE(ehdr);    // free heap allocated elf-header container
    SFREE(phdr);    // free heap allocated program header container
    close(fd);      // close file that reading executable
    if (status != 0 )
        ERROR("failed while trying to munmap virtual address allocated by mmap", -1);
}

void load_and_run_elf(char* elf_executable_i386_mle)
{
    // scope open elf executable i386 arch and little-endian format in read-only mode.
    fd = open(elf_executable_i386_mle, O_RDONLY);
    int elf32_ehdr_size = sizeof(Elf32_Ehdr);
    int elf32_phdr_size = sizeof(Elf32_Phdr);

    // make space to load byte-ordered elf header in allocated memory and deal with it.
    ehdr = (Elf32_Ehdr *)malloc(elf32_ehdr_size);
    if (!ehdr) CFARF("[ERROR] Failed to allocate memory for Elf32_Ehdr, try again.", fd);

    // read file and load byte=ordered elf header into the allocated space previously allocated.
    int ehdr_status = read(fd, ehdr, elf32_ehdr_size);
    if (ehdr_status != elf32_ehdr_size) CFARF("[ERROR] failed to read Elf32_Ehdr*.", fd);

    // we can proceed on fact that if we shift file pointer by size of Elf-header we are now
    // at start of program headers and each entity size can be taken from @var ehdr->e_phentsize.
    //                    +-------------------------+ -> fd at reset/begin
    //                    |                         |
    //                    |        ElfHEADER        |
    //                    |                         |
    //                    +-------------------------+ -> fd after reading ehdr
    //                 +-{|        ProHEADER        |
    //                 |  +-------------------------+ 
    //  PROG HEADER <--+-{|        ProHEADER        |
    //                 |  + ... ... ... ... ... ... +
    //                 +-{|                         |
    // clearly we can proceed with reading each segment as fd is at top of phdr after reading ehdr.
    // i dont think this is safe to do with this approach, what if we end up reading garbage.
    // reset the fd and then go to offset provided by ehdr->e_phoff and then read each segment.
    int program_header_offset = ehdr->e_phoff; lseek(fd, program_header_offset, SEEK_SET);

    // make space to load byte-ordered program header in allocated memory and deal with it.
    phdr = (Elf32_Phdr *)malloc(elf32_phdr_size);
    if (!ehdr) CFARF("[ERROR] Failed to allocate memory for Elf32_Phdr, try again.", fd);

    // read all segments and choose the one containing the entrypoint and store that segment.
    for (int i = 0; i < ehdr->e_phnum; i++){
        int phdr_status = read(fd, phdr, elf32_phdr_size);
        if (phdr_status != elf32_phdr_size) CFARF("[ERROR] failed to read Elf32_Phdr*.", fd);

        // break if virtual address of entrypoint is within the scope of current segment.
        int is_loadable = phdr->p_type == PT_LOAD;
        int is_gt_range = phdr->p_vaddr <= ehdr->e_entry;
        int is_lt_range = ehdr->e_entry <= (phdr->p_vaddr + phdr->p_filesz);
        if (is_loadable && is_gt_range && is_lt_range)
            break;
    }
    
    // create a memory_space with execution permission and valid permissions to run _start.
    vm = mmap(NULL, phdr->p_memsz, PROT_READ|PROT_EXEC|PROT_WRITE, MAP_ANONYMOUS|MAP_PRIVATE, 0, 0);
    if (!vm) CFARF("[ERROR] cannot allocate memory with read, write and exec permissions.", fd);

    // load instructions from executable to vistually-allocated memory with exec permission
    lseek(fd, phdr->p_offset, SEEK_SET); read(fd, vm, phdr->p_filesz);

    // construct payload/hook to funstion int _start(void) { ... } to execute the payload.
    int (*_start)(void) = (int (*)(void))(vm + (ehdr->e_entry - phdr->p_vaddr));
    printf("User _start return value = %d\n", _start());
}