#include "allocator.h"

uInt32 align_page_address(uInt32 vaddr){
    return vaddr & virtual_page_alignment_mask;
}

uInt32 align_offs_address(uInt32 vaddr){
    return vaddr & virtual_offset_address_mask;
}

uInt32 allocate_page(uInt32 vaddr){

    uInt32 vaddr_base  = align_page_address(vaddr);
    uInt32 vaddr_bound = 0x1000;
    uInt32 isallocated = 0x0000;

    for (uInt32 i=0; i < ehdr->e_phnum; i++){
        uInt32 is_gt_base = phdr[i].p_vaddr <= vaddr;
        uInt32 is_st_next = phdr[i].p_vaddr 
                          + phdr[i].p_memsz >= vaddr;
        
        if (!is_gt_base || !is_st_next)
            continue;
        
        sInt32 file_offset = vaddr_base   - phdr[i].p_vaddr;
        sInt32 file_remain = phdr[i].p_filesz - file_offset;
        sInt32 is_bss_segm = file_remain <= 0 ? 0x01 : 0x00;
        if (file_remain > 0x1000) file_remain = vaddr_bound;

        uInt32 protection = 0;
        protection |= (phdr[i].p_flags & PF_R) ? PROT_READ  : 0;
        protection |= (phdr[i].p_flags & PF_W) ? PROT_WRITE : 0;
        protection |= (phdr[i].p_flags & PF_X) ? PROT_EXEC  : 0;

        uInt32 preloadprot = PROT_READ|PROT_WRITE|PROT_EXEC;

        void* allocate = mmap((void*)vaddr_base, 0x1000, preloadprot, MAP_ANONYMOUS|MAP_PRIVATE|MAP_FIXED, 0, 0);
        if (!allocate) CFARF("[ERROR] cannot allocate memory with predefined R?W?X? permissions.", fd);
        if (is_bss_segm == 0x00){
            sInt32 file_seek_positions = phdr[i].p_offset + file_offset;

            lseek(fd, file_seek_positions, SEEK_SET);
            read(fd, (void*)vaddr_base, file_remain);
        }

        if (mprotect((void*)vaddr_base, 0x1000, protection) == -1)
            CFARF("[ERROR] protection for newly allocated page failed to add defined permissions.", fd);
        

        isallocated = 0x0001;

        uInt32 page_index = (vaddr_base - phdr[i].p_vaddr) / 0x1000;
        uInt32 filled_bit = phdr[i].p_memsz - 0x1000*page_index;
               filled_bit = (0x1000*(page_index + 1) <= phdr[i].p_memsz) ? 0x1000 : filled_bit; 
        
        if (isallocated == 1)
            page_inter_frags += 0x1000 - filled_bit;
    }
    
    if (isallocated == 0)
        CFARF("[ERROR] allocation not possible for program, element not present in address space.", fd);

    page_alloc_count += isallocated;
    return 0;
}
