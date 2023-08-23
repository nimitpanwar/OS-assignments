#include "loader.h"

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
    if (phdr || ehdr) {
        free(phdr);
        free(ehdr);
    }
    if (fd != -1) {
        close(fd);
    }
}


/*
 * Load and run the ELF executable file
 */
void load_and_run_elf(char** exe) {
    fd = open(exe[1], O_RDONLY);
    if (fd == -1) {
        perror("Error opening ELF file");
        exit(EXIT_FAILURE);
    }
    off_t fileSize = lseek(fd, 0, SEEK_END);
    if (fileSize == -1) {
        perror("Error getting file size");
        exit(EXIT_FAILURE);
    }
    void *content = malloc(fileSize);
    if (content == NULL) {
        perror("Error allocating memory");
        exit(EXIT_FAILURE);
    }
    lseek(fd, 0, SEEK_SET);
    ssize_t bytesread= read(fd, content, fileSize);
    if (bytesread == -1) {
        perror("Error reading file");
        free(content);
        exit(EXIT_FAILURE);
    }
    

    ehdr = (Elf32_Ehdr *)content;
    unsigned char *fileData = (unsigned char *)content;
    memcpy(ehdr,fileData,sizeof(Elf32_Ehdr));
    

    off_t phOffset = ehdr->e_phoff;
    size_t phEntrySize = ehdr->e_phentsize;
    size_t numPhEntries = ehdr->e_phnum;
    phdr = (Elf32_Phdr *)malloc(phEntrySize*numPhEntries);
    ssize_t ph_read = pread(fd, phdr, phEntrySize*numPhEntries, phOffset);

    Elf32_Phdr *entry_phdr = NULL;
    for (size_t i = 0; i < numPhEntries; ++i) {
        if (phdr[i].p_type == PT_LOAD && ehdr->e_entry >= phdr[i].p_vaddr && ehdr->e_entry < phdr[i].p_vaddr + phdr[i].p_memsz) {
            if(phdr[i].p_type!=PT_NULL){
                entry_phdr=&phdr[i];
                break;
            }
        }
    }

    void *source_addr = (void *)((uintptr_t)fileData + (entry_phdr->p_offset));

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

    if (close(fd) == -1) {
        perror("Error closing file");
        free(content);
        exit(EXIT_FAILURE);
    }
}
    
int main(int argc, char** argv) 
{
  if(argc != 2) {
    printf("Usage: %s <ELF Executable> \n", argv[0]);
    exit(1);
  }
  
  // 1. carry out necessary checks on the input ELF file
  // 2. passing it to the loader for carrying out the loading/execution
  load_and_run_elf(argv); // Corrected line
  
  // 3. invoke the cleanup routine inside the loader  
  loader_cleanup();
  return 0;
}
