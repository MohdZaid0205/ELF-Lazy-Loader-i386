#include "loader.h"

// To display exceptions and exiting at runtime defined macros  are to be used along
// if statements or assertions to make it easer to log exceptions and exit at fatal flaw.
#define ERROR(message, x) { printf(message);  }             // to display error message.
#define FATAL(message, x) { ERROR(message, x); exit(x); }   // to display and exit fatal.
#define CFARF(message, f) { close(f); FATAL(message,-1);}   // to close file and raise.


int main(int argc, char** argv) {
    // if not filename or more than one file names are passed in executable.
    if(argc < 2) FATAL("[ERROR] perhaps you forgot to pass ELF file name.", -1);
    if(argc > 2) FATAL("[ERROR] passed too many files, expected only ONE.", -1);

    // check if given file exists or not assuming argv[1] is relative file name.
    char* elf_executable_i386 = argv[1];
    if(access(elf_executable_i386, F_OK) != 0)
        FATAL("[ERROR] provided file does not !exist. provide valid file.", -1);
    // check if we have reading permission on file or else exit by raising.
    if(access(elf_executable_i386, R_OK) != 0)
        FATAL("[ERROR] file exists but !read permission is denied for it.", -1);
    // file exists and we have read permission to file and we need to validate elf.

    int fd = open(elf_executable_i386, O_RDONLY);
    if (fd == -1) FATAL("[ERROR] failed to !open provided file for reads.", -1);
    
    // validate if file is a valid elf file and compiled in i386 and little-endian.
    // Elf32_Ehdr* elf_header_i386 = malloc(sizeof(Elf32_Ehdr));
    // int e_status = read(fd, elf_header_i386, sizeof(Elf32_Ehdr));
    // if (e_status != sizeof(Elf32_Ehdr))
    //     FATAL("[ERROR] elf header may be curropted, cannot read Elf_Ehdr*.", -1);
    // #error then compare magic number and class with that required for i386 and proceed.
    // #error this may load insufficent data for x64 and end up failing before checks.

    // load only the magic bits and read necessary information and close file discriptor.
    unsigned char elf_identifier[EI_NIDENT];
    int ident_status = read(fd, &elf_identifier, EI_NIDENT);
    if (ident_status != EI_NIDENT)
        CFARF("[ERROR] cannot read 16bytes of magic number from given src.", fd);
    if (elf_identifier[EI_MAG0] != ELFMAG0 ||
        elf_identifier[EI_MAG1] != ELFMAG1 ||
        elf_identifier[EI_MAG2] != ELFMAG2 ||
        elf_identifier[EI_MAG3] != ELFMAG3 )
        CFARF("[ERROR] invalid magic for elf was found in the source given", fd);
    if (elf_identifier[EI_CLASS] != ELFCLASS32)
        CFARF("[ERROR] given source was not compiled with flag -m32 | i386", fd);
    if (elf_identifier[EI_DATA] != ELFDATA2LSB)
        CFARF("[ERROR] source is big-endian not valid for little-endian PL", fd);
    // these checks have validated that our executable is i386 and little-endian and valid.
    close(fd);

    // load elf and execute _start using payload or hook with _start in file.
    load_and_run_elf(elf_executable_i386); loader_cleanup();

    return 0;
}
