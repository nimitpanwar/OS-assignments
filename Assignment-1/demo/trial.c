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

int main()//int argc, char** argv) 
{
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

    off_t phOffset = ehdr->e_phoff;
    size_t phEntrySize = ehdr->e_phentsize;
    size_t numPhEntries = ehdr->e_phnum;
    phdr = (Elf32_Phdr *)malloc(phEntrySize*numPhEntries);
    ssize_t ph_read = pread(fd, phdr, phEntrySize*numPhEntries, phOffset);
    // for (size_t i = 0; i < 2; ++i) {
    //     printf("p_type=%08X, p_vaddr=%08X\n", phdr[i].p_type , phdr[i].p_mem);
    // }
    void* virtual_mem;
    virtual_mem=mmap(NULL,phdr[1].p_memsz,PROT_READ|PROT_WRITE|PROT_EXEC,MAP_ANONYMOUS|MAP_PRIVATE, 0,0);

    void *source_addr = (void *)((uintptr_t)fileData + phdr[1].p_offset);
    void *destination_addr = virtual_mem;
    size_t segment_size = phdr[1].p_filesz;
    memcpy(destination_addr, source_addr, segment_size);

    off_t entry_offset = ehdr->e_entry - phdr[1].p_vaddr;
    void *entry_addr = (void *)((uintptr_t)virtual_mem + entry_offset);
    typedef int (*StartFunction)(void);
    StartFunction _start=(StartFunction)entry_addr;
    int result = _start();
    printf("User _start return value = %d\n",result);
    return 0;
}
