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
    // Step 1: Open and read the ELF binary file
    int fd = open(exe[1], O_RDONLY);
    if (fd == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    off_t ELF_size = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);

    void *content = malloc(ELF_size);
    ssize_t bytesread = read(fd, content, ELF_size);
    close(fd);

    // Step 2: Parse ELF header and program header table
    Elf32_Ehdr *ehdr = (Elf32_Ehdr *)content;
    unsigned char *ELF_data = (unsigned char *)content;

    size_t numEntries = ehdr->e_phnum;
    off_t phOffset = ehdr->e_phoff;
    size_t phEntrySize = ehdr->e_phentsize;

    Elf32_Phdr *phdr = (Elf32_Phdr *)malloc(phEntrySize * numEntries);
    fd = open(exe[1], O_RDONLY); // Reopen the file to read program headers
    ssize_t ph_read = pread(fd, phdr, phEntrySize * numEntries, phOffset);
    close(fd);

    // Step 3: Locate and allocate memory for the loadable segment
    Elf32_Phdr *reqPhdr = NULL;
    for (size_t i = 0; i < numEntries; ++i) {
        if (phdr[i].p_type == PT_LOAD && ehdr->e_entry >= phdr[i].p_vaddr && ehdr->e_entry < phdr[i].p_vaddr + phdr[i].p_memsz) {
            reqPhdr = &phdr[i];
            break;
        }
    }

    if (reqPhdr == NULL) {
        fprintf(stderr, "Required program header not found\n");
        exit(EXIT_FAILURE);
    }

    void *reqAddr = ELF_data + reqPhdr->p_offset;

    // Allocate memory with proper protection flags using mmap
    void *virtual_mem = mmap(NULL, reqPhdr->p_memsz, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_ANONYMOUS | MAP_PRIVATE, 0, 0);
    if (virtual_mem == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }

    // Copy segment content into allocated memory
    memcpy(virtual_mem, reqAddr, reqPhdr->p_filesz);

    // Step 4: Resolve entry point function and execute it
    typedef int (*StartFunction)(void);
    StartFunction _start = (StartFunction)(virtual_mem + (ehdr->e_entry - reqPhdr->p_vaddr));
    int result = _start();

    // Step 5: Clean up allocated memory and resources
    printf("_start return value = %d\n", result);
    munmap(virtual_mem, reqPhdr->p_memsz);
    free(content);
    free(phdr);
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
