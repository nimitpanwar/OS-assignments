#include "loader.h"

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
    off_t fileSize = lseek(fd, 0, SEEK_END);
    void *content = malloc(fileSize);
    lseek(fd, 0, SEEK_SET);
    ssize_t bytesread= read(fd, content, fileSize);
    // print_memory_content(content, fileSize);
    ehdr = (Elf32_Ehdr *)content;
    unsigned char *fileData = (unsigned char *)content;
    memcpy(ehdr,fileData,sizeof(Elf32_Ehdr));
    
    // printf("%04X\n", ehdr->e_ident[0]);
    // printf("fileData: %p\n", (void *)fileData);
    off_t phOffset = ehdr->e_phoff;
    size_t phEntrySize = ehdr->e_phentsize;
    size_t numPhEntries = ehdr->e_phnum;
    phdr = (Elf32_Phdr *)malloc(phEntrySize*numPhEntries);
    ssize_t ph_read = pread(fd, phdr, phEntrySize*numPhEntries, phOffset);
    // for (size_t i = 0; i < 2; ++i) {
    //     printf("p_type=%08X, p_vaddr=%08X\n", phdr[i].p_type , phdr[i].p_memsz);
    // // }

    Elf32_Phdr *entry_phdr = NULL;
    for (size_t i = 0; i < numPhEntries; ++i) {
        if (phdr[i].p_type == PT_LOAD && ehdr->e_entry >= phdr[i].p_vaddr && ehdr->e_entry < phdr[i].p_vaddr + phdr[i].p_memsz) {
            if(phdr[i].p_type!=PT_NULL){
                entry_phdr=&phdr[i];
                break;
            }
        }
    }
    // printf("p_offset=%08X, p_vaddr=%08X\n",phdr[i].p_offset,phdr[i].p_vaddr);

    // void* source_addr = (void*)((uintptr_t)fileData + phdr[i].p_offset);
    void *source_addr = (void *)((uintptr_t)fileData + (entry_phdr->p_offset));

    // printf("source address: %p\n", (void *)source_addr);
    void *virtual_mem;
    virtual_mem=mmap(NULL,entry_phdr->p_memsz,PROT_READ|PROT_WRITE|PROT_EXEC,MAP_ANONYMOUS|MAP_PRIVATE,0,0);
    if (virtual_mem == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }

    
    memcpy(virtual_mem, source_addr, entry_phdr->p_filesz);


    typedef int (*StartFunction)(void);
    StartFunction _start = (StartFunction)(virtual_mem+(ehdr->e_entry-entry_phdr->p_vaddr));
    int result = _start();
    printf("_start return value = %d\n", result);

    // munmap(virtual_mem, phdr[i].p_memsz);
}



int main(int argc, char** argv) 
{
  if(argc != 2) {
    printf("Usage: %s <ELF Executable> \n",argv[0]);
    exit(1);
  }
  // 1. carry out necessary checks on the input ELF file
  // 2. passing it to the loader for carrying out the loading/execution
  load_and_run_elf(argv);
  // 3. invoke the cleanup routine inside the loader  
  loader_cleanup();
  return 0;
}

