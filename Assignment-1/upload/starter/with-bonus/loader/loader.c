#include "loader.h"

Elf32_Ehdr *ehdr;
Elf32_Phdr *phdr;
int fd;

void loader_cleanup() {
    if (phdr || ehdr) {
        free(phdr);
        free(ehdr);
    }
    if (fd != -1) {
        close(fd);
    }
}

void load_and_run_elf(char** exe) {
    // Step 1: Open and read the ELF binary file
    int fd = open(exe[1], O_RDONLY); // Open the ELF binary file for reading
    if (fd == -1) {
        perror("open"); // Print an error message if opening the file fails
        exit(EXIT_FAILURE); // Exit the program with failure status
    }

    // Get the size of the ELF binary file
    off_t ELF_size = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET); // Reset file pointer to the beginning

    // Allocate memory to store the content of the ELF binary
    void *content = malloc(ELF_size);

    // Read the content of the file into the allocated memory
    ssize_t bytesread = read(fd, content, ELF_size);
    close(fd); // Close the file after reading

    // Step 2: Parse ELF header and program header table
    Elf32_Ehdr *ehdr = (Elf32_Ehdr *)content; // Cast the content to ELF header struct
    unsigned char *ELF_data = (unsigned char *)content; // Cast content to unsigned char pointer

    // Extract program header information
    size_t numEntries = ehdr->e_phnum; // Number of program header entries
    off_t phOffset = ehdr->e_phoff; // Offset of program header table
    size_t phEntrySize = ehdr->e_phentsize; // Size of each program header entry

    // Allocate memory to store program header entries
    Elf32_Phdr *phdr = (Elf32_Phdr *)malloc(phEntrySize * numEntries);

    // Reopen the file to read program headers
    fd = open(exe[1], O_RDONLY);
    ssize_t ph_read = pread(fd, phdr, phEntrySize * numEntries, phOffset);
    close(fd);

    // Step 3: Locate and allocate memory for the loadable segment
    Elf32_Phdr *reqPhdr = NULL; // Placeholder for the required program header
    for (size_t i = 0; i < numEntries; ++i) {
        // Find the program header that contains the entry point address
        if (phdr[i].p_type == PT_LOAD && ehdr->e_entry >= phdr[i].p_vaddr && ehdr->e_entry < phdr[i].p_vaddr + phdr[i].p_memsz) {
            reqPhdr = &phdr[i];
            break;
        }
    }

    if (reqPhdr == NULL) {
        fprintf(stderr, "Required program header not found\n"); // Print an error if the required header is not found
        exit(EXIT_FAILURE);
    }

    // Calculate the address where the executable segment content starts
    void *reqAddr = ELF_data + reqPhdr->p_offset;

    // Allocate memory with proper protection flags using mmap
    void *virtual_mem = mmap(NULL, reqPhdr->p_memsz, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_ANONYMOUS | MAP_PRIVATE, 0, 0);
    if (virtual_mem == MAP_FAILED) {
        perror("mmap"); // Print an error message if mmap fails
        exit(EXIT_FAILURE);
    }

    // Copy segment content into the allocated memory
    memcpy(virtual_mem, reqAddr, reqPhdr->p_filesz);

    // Step 4: Resolve entry point function and execute it
    typedef int (*StartFunction)(void);
    StartFunction _start = (StartFunction)(virtual_mem + (ehdr->e_entry - reqPhdr->p_vaddr)); // Calculate the address of the entry point function
    int result = _start(); // Call the entry point function and store its result

    // Step 5: Clean up allocated memory and resources
    printf("_start return value = %d\n", result); // Print the result of the entry point function
    munmap(virtual_mem, reqPhdr->p_memsz); // Unmap the allocated memory
}

