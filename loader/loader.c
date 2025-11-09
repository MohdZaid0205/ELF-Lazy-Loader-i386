#include "loader.h"

// important headers and file to be loaded, executed and cleaned in corresponding func.
Elf32_Ehdr* ehdr;   // stores elf header, see [>_] for informations regarding readings.  
Elf32_Phdr* phdr;   // stores program header, see [>_] for informations on structure.
sInt32        fd;   // stored file-descriptor, to open/close and read/seek when needed.


// global variables to count required loader informations regarding paging and falults.
volatile uInt32 page_fault_count = 0;   // number of page faults encountered by loader.
volatile uInt32 page_alloc_count = 0;   // number of times new page was allocated for .
volatile uInt32 page_inter_frags = 0;   // total internal fragmentation over all pages.

// helper masking bits specifically for page alignment and retreiving offset of address.
// these constants are only viable for page size = 0x1000 ie size=4kB
const uInt32 virtual_page_alignment_mask = 0xfffff000;
const uInt32 virtual_offset_address_mask = 0x00000fff;

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
    if (status != 0 )
        ERROR("[ERROR] failed while trying to munmap virtual address allocated by mmap", -1);
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
        int phdr_status = read(fd, &phdr[i], elf32_phdr_size);
        if (phdr_status != elf32_phdr_size) CFARF("[ERROR] failed to read Elf32_Phdr*.", fd);
    }
    
    // in order to lazily load pages, we need to map exactly what section and what page is to
    // be loaded from File, following is a calculation that is necessary for page fault resolution.
    // 
    // Program Headers:
    //      Type           Offset   VirtAddr   PhysAddr   FileSiz MemSiz  Flg Align
    //      LOAD           0x000000 0x08048000 0x08048000 0x00118 0x00118 R   0x1000
    //      LOAD           0x001000 0x08049000 0x08049000 0x00046 0x00046 R E 0x1000
    //      LOAD           0x002000 0x0804a000 0x0804a000 0x00074 0x00074 R   0x1000
    //      NOTE           0x0000f4 0x080480f4 0x080480f4 0x00024 0x00024 R   0x4
    //      GNU_EH_FRAME   0x002000 0x0804a000 0x0804a000 0x0001c 0x0001c R   0x4
    //      GNU_STACK      0x000000 0x00000000 0x00000000 0x00000 0x00000 RW  0x10
    //
    // in order to resolve page fault occured due to unallocated virtual address, we take VAddr
    // and do the following calculation, to verify what should be page address to be allocated
    // and allocate fixes pagesize=4kb with required permissions to page.
    //
    //      VIR_PAGE_ADDR = VAddr & (0xffffffff ^ 0x0fff)
    //      VIR_OFFSET_IN = VAddr & 0x0fff < 0x1000 ? VAddr & (0x0fff) : ERR
    //
    // then allocate exactly one page with provided base = VIR_PAGE_ADDR and bound = 0x1000
    // base = VIR_PAGE_ADDR as in i386, VIR and PHY ADDR are considerd same and within LIMIT
    
    // createa a segmentaion fault handler and assign it to sigsegv, to handle segmentation.
    struct sigaction sa; memset(&sa, 0, sizeof(sa));
    sa.sa_sigaction = segmentation_fault_handler;
    sa.sa_flags = SA_SIGINFO; sigemptyset(&sa.sa_mask);

    if (sigaction(SIGSEGV, &sa, NULL) == -1)
        FATAL("[ERROR] sigaction failed failed to assign function for SIG SEGMENTATION.", -1);

    // construct payload/hook to funstion int _start(void) { ... } to execute the payload.
    sInt32 (*_start)(void) = (sInt32 (*)(void))((void*)ehdr->e_entry);
    printf("User _start return value = %d\n", _start());

    DEBUG(("total PAGE FAULTS: %d  \n", page_fault_count));
    DEBUG(("total PAGE ALLOCS: %d  \n", page_alloc_count));
    DEBUG(("total IN-FRAGMENT: %dKB\n", page_inter_frags/0x1000));
}
