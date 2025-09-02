#include "loader.h"

// define aliases for types (just for cleanliness purposes). no cahnge in workings.
#define uInt32 uint32_t
#define sInt32 int32_t

// important headers and file to be loaded, executed and cleaned in corresponding functions
Elf32_Ehdr* ehdr;   // stores elf header, see [>_] for informations regarding readings.  
Elf32_Phdr* phdr;   // stores program header, see [>_] for informations on structure.
sInt32        fd;   // stored file-descriptor, to open/close and read/seek when needed.

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
    
    for (int i = 0; i < ehdr->e_phnum; i++){
        Elf32_Phdr* active = phdr + (i*sizeof(Elf32_Phdr));
        if (active->p_type != PT_LOAD) continue;
        
        // align items to nearest page boundries, (finding page boundries).
        uInt32 bound_str = active->p_vaddr - (active->p_vaddr % active->p_align);
        uInt32 bound_end = active->p_vaddr + active->p_memsz;
        bound_end = bound_end - (bound_end % active->p_align) + active->p_align;
        uInt32 bound_len = bound_end - bound_str;

        // release virually allocated pages
        int deallocated = munmap((void*)bound_str, bound_len);
        if (deallocated != 0) CFARF("[ERROR] cannot de-allocate memory previously paged.", fd);
    }
    
    SFREE(ehdr);    // free heap allocated elf-header container
    SFREE(phdr);    // free heap allocated programme-header container
    close(fd);      // close file that reading executable
}

void load_and_run_elf(char* elf_executable_i386_mle)
{
    // scope open elf executable i386 arch and little-endian format in read-only mode.
    fd = open(elf_executable_i386_mle, O_RDONLY);
    uInt32 elf32_ehdr_size = sizeof(Elf32_Ehdr);
    uInt32 elf32_phdr_size = sizeof(Elf32_Phdr);

    // make space to load byte-ordered elf header in allocated memory and deal with it.
    ehdr = (Elf32_Ehdr *)malloc(elf32_ehdr_size);
    if (!ehdr) CFARF("[ERROR] Failed to allocate memory for Elf32_Ehdr, try again.", fd);

    // read file and load byte=ordered elf header into the allocated space previously allocated.
    int ehdr_status = read(fd, ehdr, elf32_ehdr_size);
    if (ehdr_status != elf32_ehdr_size) CFARF("[ERROR] failed to read Elf32_Ehdr*.", fd);

    // no need to execute if given source is not an executable file, ignore the source.
    if ((ehdr->e_type & (ET_EXEC|ET_DYN)) == 0)
        CFARF("[ERROR] given elf-i386 file is not an executable file, !EXEC.", fd);

    // on some modern systems even dynamic files can execute properly, it is intended feature.
    if (ehdr->e_type == ET_DYN)
        ERROR("[WARNS] given elf-i386 is DYN not EXEC, still proceeding with execution.", -1);

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
    uInt32 program_header_offset = ehdr->e_phoff; lseek(fd, program_header_offset, SEEK_SET);

    // make space to load byte-ordered program header in allocated memory and deal with it.
    phdr = (Elf32_Phdr *)malloc(elf32_phdr_size*ehdr->e_phnum);
    if (!ehdr) CFARF("[ERROR] Failed to allocate memory for Elf32_Phdr, try again.", fd);

    // read all segments and store header for that segments for further use in programme.
    for (uInt32 i = 0; i < ehdr->e_phnum; i++){
        int phdr_status = read(fd, phdr + (i*elf32_phdr_size), elf32_phdr_size);
        if (phdr_status != elf32_phdr_size) CFARF("[ERROR] failed to read Elf32_Phdr*.", fd);
    }
    
    // for each segment that need to be loaded into virtual address space, allocate segment info
    for (int i = 0; i < ehdr->e_phnum; i++){
        Elf32_Phdr* active = phdr + (i*elf32_phdr_size);
        if (active->p_type != PT_LOAD) continue;
        
        // align items to nearest page boundries, (finding page boundries).
        uInt32 bound_str = active->p_vaddr - (active->p_vaddr % active->p_align);
        uInt32 bound_end = active->p_vaddr + active->p_memsz;
        bound_end = bound_end - (bound_end % active->p_align) + active->p_align;
        uInt32 bound_len = bound_end - bound_str;

        // get feasable protections from program header and convert them to mmap permission types.
        uInt32 protections = PROT_READ|PROT_EXEC|PROT_WRITE;

        lseek(fd, active->p_offset, SEEK_SET);
        // allocate virtual address and handle failed allocation cases, follow up by copying data.
        void* allocated = mmap((void*)bound_str, bound_len, protections, MAP_ANONYMOUS|MAP_PRIVATE, 0, 0);
        if (!allocated) CFARF("[ERROR] cannot allocate memory with read, write and exec permissions.", fd);
        lseek(fd, active->p_offset, SEEK_SET); read(fd, allocated, phdr->p_filesz);
        
        // this is to initialize .bss section with value 0, and overrite if in any case anything had been written.
        if (active->p_memsz > active->p_filesz) 
            memset((void*)(active->p_vaddr + active->p_filesz), 0, active->p_memsz - active->p_filesz);
    }

    // construct payload/hook to funstion int _start(void) { ... } to execute the payload.
    sInt32 (*_start)(void) = (sInt32 (*)(void))((void*)ehdr->e_entry);
    printf("User _start return value = %d\n", _start());
}