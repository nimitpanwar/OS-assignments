#include "loader.h"
#include <signal.h>
#include <ucontext.h>

Elf32_Ehdr *ehdr;
Elf32_Phdr *phdr;
int fd;


static void my_handler(int signum, siginfo_t* si, void* vcontext) {
    if (signum == SIGSEGV) {
        uintptr_t segfault_address = (uintptr_t) si->si_addr;

        
        Elf32_Phdr *reqPhdr = NULL;
        for (size_t i = 0; i < ehdr->e_phnum; ++i) {
            if (phdr[i].p_type == PT_LOAD && segfault_address >= phdr[i].p_vaddr && segfault_address < phdr[i].p_vaddr + phdr[i].p_memsz) {
                if(phdr[i].p_type!=PT_NULL){
                    reqPhdr=&phdr[i];
                    break;
                }
            }
        }
        
        void* reqAddr =(void *)((uintptr_t)ehdr + (reqPhdr->p_offset));

        void *allocated_memory = mmap((void*)segfault_address,reqPhdr->p_memsz,PROT_READ|PROT_WRITE|PROT_EXEC,MAP_ANONYMOUS|MAP_PRIVATE,0,0);
        if (allocated_memory == MAP_FAILED) {
            perror("mmap");
            exit(1);
        }
        memcpy(allocated_memory,reqAddr,reqPhdr->p_filesz);
    }
}






void load_and_run_elf(char** exe) {


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


    typedef int (*StartFunction)(void);
    StartFunction _start = (StartFunction)((uintptr_t)ehdr->e_entry);
    int result = _start();

    printf("_start return value = %d\n", result);

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
  
    load_and_run_elf(argv);

    return 0;
}