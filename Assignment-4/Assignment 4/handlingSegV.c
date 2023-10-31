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


void segfault_handler(int signo, siginfo_t *si, void *ctx) {
    if (si->si_signo == SIGSEGV) {
        void *fault_address = si->si_addr;
        printf("Segmentation fault at address: %p\n", fault_address);

        // Allocate a page of memory at the fault address
        void *allocated_memory = mmap(fault_address,getpagesize(),PROT_READ|PROT_WRITE|PROT_EXEC,MAP_ANONYMOUS|MAP_PRIVATE,0,0);
        
        if (allocated_memory == MAP_FAILED) {
            perror("mmap");
            exit(1);
        }

        printf("Allocated memory at address: %p\n", allocated_memory);

        // Now, you can copy the appropriate data into allocated_memory

        // Resume execution at the fault address
        memcpy(allocated_memory, "Hello, World!", 20);
    }
}

int main() {
    struct sigaction sa;
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = segfault_handler;

    if (sigaction(SIGSEGV, &sa, NULL) == -1) {
        perror("sigaction");
        return 1;
    }

    // Access an address to trigger a segmentation fault
    char *ptr = NULL;
    *ptr = 'A';  // This will cause a segmentation fault

    return 0;
}