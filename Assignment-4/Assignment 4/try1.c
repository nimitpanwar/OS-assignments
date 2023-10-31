#include "loader.h"
#include <signal.h>
#include <ucontext.h>

size_t memsize;
void *reqAddr;

int seg=0;

typedef int (*StartFunction)(void);

int diff;

static void my_handler(int signum, siginfo_t* si, void* vcontext) {
    if (signum == SIGSEGV) {
        void* faulting_address = si->si_addr;
        printf("faulting address = %p\n",faulting_address);
        void *allocated_memory = mmap(faulting_address,memsize,PROT_READ|PROT_WRITE|PROT_EXEC,MAP_ANONYMOUS|MAP_PRIVATE,0,0);
        if (allocated_memory == MAP_FAILED) {
            perror("mmap");
            exit(1);
        }
        printf("Allocated memory at address: %p\n", allocated_memory);
        memcpy(allocated_memory,reqAddr,memsize);
        StartFunction _start = (StartFunction)(allocated_memory+(diff));
        int result=_start();
        printf("%d\n",result);
        // exit(0);
    }
}


Elf32_Ehdr *ehdr;
Elf32_Phdr *phdr;
int fd;


/*
 * release memory and other cleanups
 */ 
void loader_cleanup() {
  
}

/*
 * Load and run the ELF executable file
 */
void load_and_run_elf(char** exe) {
  // 1. Load entire binary content into the memory from the ELF file.
  // 2. Iterate through the PHDR table and find the section of PT_LOAD 
  //    type that contains the address of the entrypoint method in fib.c
  // 3. Allocate memory of the size "p_memsz" using mmap function 
  //    and then copy the segment content
  // 4. Navigate to the entrypoint address into the segment loaded in the memory in above step
  // 5. Typecast the address to that of function pointer matching "_start" method in fib.c.
  // 6. Call the "_start" method and print the value returned from the "_start"

    fd = open(exe[1], O_RDONLY);
    off_t elfSize = lseek(fd, 0, SEEK_END);
    void *content = malloc(elfSize);
    lseek(fd, 0, SEEK_SET);
    ssize_t bytesread= read(fd, content, elfSize);
    ehdr = (Elf32_Ehdr *)content;
    unsigned char *elfData = (unsigned char *)content;
    memcpy(ehdr,elfData,sizeof(Elf32_Ehdr));
    

    size_t numEntries = ehdr->e_phnum;
    off_t phOffset = ehdr->e_phoff;
    size_t phEntrySize = ehdr->e_phentsize;
    phdr = (Elf32_Phdr *)malloc(phEntrySize*numEntries);
    ssize_t ph_read = pread(fd, phdr, phEntrySize*numEntries, phOffset);

    Elf32_Phdr *reqPhdr = NULL;
    for (size_t i = 0; i < numEntries; ++i) {
        if (phdr[i].p_type == PT_LOAD && ehdr->e_entry >= phdr[i].p_vaddr && ehdr->e_entry < phdr[i].p_vaddr + phdr[i].p_memsz) {
            if(phdr[i].p_type!=PT_NULL){
                reqPhdr=&phdr[i];
                break;
            }
        }
    }

    reqAddr = (void *)((uintptr_t)elfData + (reqPhdr->p_offset));
    memsize= reqPhdr->p_memsz;

    diff=ehdr->e_entry-reqPhdr->p_vaddr;
    // void *virtual_mem;
    // virtual_mem=mmap(NULL,reqPzhdr->p_mems,PROT_READ|PROT_WRITE|PROT_EXEC,MAP_ANONYMOUS|MAP_PRIVATE,0,0);
    // if (virtual_mem == MAP_FAILED) {
    //     perror("mmap");
    //     exit(EXIT_FAILURE);
    // }

    
    // memcpy(virtual_mem, reqAddr, reqPhdr->p_filesz);
   

    StartFunction _start = (StartFunction)(content+ehdr->e_entry);
    int result = _start();
    // if(seg==1){
    //     printf("Yes\n");
    // }
    printf("_start return value = %d\n", result);

    // munmap(virtual_mem, phdr[i].p_memsz);
}



int main(int argc, char** argv) 
{
    if(argc != 2) {
        printf("Usage: %s <ELF Executable> \n",argv[0]);
        exit(1);
    }
                                                                                                         
    struct sigaction sig;
    memset(&sig, 0, sizeof(sig));
    sig.sa_flags = SA_SIGINFO;
    sig.sa_sigaction = my_handler;
    sigaction(SIGSEGV, &sig, NULL);
  

    // 1. carry out necessary checks on the input ELF file
    // 2. passing it to the loader for carrying out the loading/execution
    load_and_run_elf(argv);
    // 3. invoke the cleanup routine inside the loader  
    loader_cleanup();
    return 0;
}