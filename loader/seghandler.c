#include "seghandler.h"

void segmentation_fault_handler(int sig, siginfo_t* si, void* uctx)
{
    DEBUG(("Segmentation Fault Occured due to:%d\n", (uInt32)si->si_addr));
    page_fault_count++;
    allocate_page((uInt32)si->si_addr);
}