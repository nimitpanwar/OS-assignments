#include "loader2.h"

Elf32_Ehdr *ehdr;
Elf32_Phdr *phdr;
int fd;

void print_memory_content(const void *memory, ssize_t size) {
    const unsigned char *bytes = (const unsigned char *)memory;
    
    for (size_t i = 0; i <size; ++i) {
        printf("%02X ", bytes[i]); // Print each byte as a hexadecimal value
        if ((i + 1) % 16 == 0) {
            printf("\n"); // Print a new line after every 16 bytes
        }
    }
    printf("\n");
}


/*
 * release memory and other cleanups
 */
void loader_cleanup() {
}

/*
 * Load and run the ELF executable file
 */
void load_and_run_elf(char** exe) {
}
    
int main(){    
    fd = open("fib2", O_RDONLY);
    off_t fileSize = lseek(fd, 0, SEEK_END);
    void *content = malloc(fileSize);
    lseek(fd, 0, SEEK_SET);
    ssize_t bytesread= read(fd, content, fileSize);
    // print_memory_content(content, fileSize);
    ehdr = (Elf32_Ehdr *)content;
    unsigned char *fileData = (unsigned char *)content;
    memcpy(ehdr,fileData,sizeof(Elf32_Ehdr));
    // printf("%04X\n", ehdr->e_ident[0]);
    printf("fileData: %p\n", (void *)fileData);
    off_t phOffset = ehdr->e_phoff;
    size_t phEntrySize = ehdr->e_phentsize;
    size_t numPhEntries = ehdr->e_phnum;
    phdr = (Elf32_Phdr *)malloc(phEntrySize*numPhEntries);
    ssize_t ph_read = pread(fd, phdr, phEntrySize*numPhEntries, phOffset);
    // for (size_t i = 0; i < 2; ++i) {
    //     printf("p_type=%08X, p_vaddr=%08X\n", phdr[i].p_type , phdr[i].p_memsz);
    // // }

    size_t i;
    for (i = 0; i < numPhEntries; ++i) {
        if (phdr[i].p_type == PT_LOAD && ehdr->e_entry >= phdr[i].p_vaddr && ehdr->e_entry < phdr[i].p_vaddr + phdr[i].p_memsz) {
            if(phdr[i].p_type!=PT_NULL){
                break;
            }
        }
    }
    printf("p_type=%08X, p_vaddr=%08X\n",phdr[i].p_type,phdr[i].p_vaddr);

    void* source_addr = (void*)((uintptr_t)fileData + phdr[i].p_offset);
    printf("source address: %p\n", (void *)source_addr);
    void *virtual_mem;
    virtual_mem=mmap(NULL,phdr[i].p_memsz,PROT_READ|PROT_WRITE|PROT_EXEC,MAP_ANONYMOUS|MAP_PRIVATE,0,0);
    memcpy(virtual_mem, source_addr, phdr[i].p_filesz);

    off_t entry_offset = ehdr->e_entry - phdr[i].p_vaddr;
    void *entry_addr = (void *)((uintptr_t)virtual_mem + entry_offset);

    printf("Calculated entry address: %p\n", (void *)entry_addr);

    typedef int (*StartFunction)(void);
    StartFunction _start = (StartFunction)entry_addr;
    int result = _start();
    printf("User _start return value = %d\n", result);

    // munmap(virtual_mem, phdr[i].p_memsz);
}   
