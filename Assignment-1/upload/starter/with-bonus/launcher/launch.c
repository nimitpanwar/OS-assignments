#include "/home/user/upload/starter/with-bonus/loader/loader.h"

//   snippet taken from links provided by sir.

Elf32_Ehdr *ehdr1;      
Elf32_Phdr *phdr2;      
int elf_fd;            

// Function to check if the provided ELF header is valid
int elf_check_file(Elf32_Ehdr *hdr) {
    if (!hdr) return 0;  // Check if the header is NULL
    // Compare the magic number bytes to verify ELF format
    if (hdr->e_ident[EI_MAG0] != ELFMAG0) {
        printf("ELF Header EI_MAG0 incorrect.\n");
        return 0;
    }
    if (hdr->e_ident[EI_MAG1] != ELFMAG1) {
        printf("ELF Header EI_MAG1 incorrect.\n");
        return 0;
    }
    if (hdr->e_ident[EI_MAG2] != ELFMAG2) {
        printf("ELF Header EI_MAG2 incorrect.\n");
        return 0;
    }
    if (hdr->e_ident[EI_MAG3] != ELFMAG3) {
        printf("ELF Header EI_MAG3 incorrect.\n");
        return 0;
    }
    return 1;  // The ELF header is valid
}

// Function to check if the provided ELF file is supported
// Returns 1 if supported, 0 otherwise
int elf_check_supported(Elf32_Ehdr *hdr) {
    if (!elf_check_file(hdr)) {
        printf("Invalid ELF File.\n");
        return 0;
    }
    // Check various attributes of the ELF header to determine support
    if (hdr->e_ident[EI_CLASS] != ELFCLASS32) {
        printf("Unsupported ELF File Class.\n");
        return 0;
    }
    if (hdr->e_ident[EI_DATA] != ELFDATA2LSB) {
        printf("Unsupported ELF File byte order.\n");
        return 0;
    }
    if (hdr->e_machine != EM_386) {
        printf("Unsupported ELF File target.\n");
        return 0;
    }
    if (hdr->e_ident[EI_VERSION] != EV_CURRENT) {
        printf("Unsupported ELF File version.\n");
        return 0;
    }
    if (hdr->e_type != ET_REL && hdr->e_type != ET_EXEC) {
        printf("Unsupported ELF File type.\n");
        return 0;
    }
    return 1;  // The ELF file is supported
}

// Main function that loads and runs an ELF executable
int main(int argc, char** argv) {
    // Check for correct usage
    if (argc != 2) {
        printf("Usage: %s <ELF Executable>\n", argv[0]);
        exit(1);
    }
    // Open the ELF file
    elf_fd = open(argv[1], O_RDONLY);
    // Determine the size of the ELF file
    off_t fileSize = lseek(elf_fd, 0, SEEK_END);
    // Allocate memory to hold the entire file content
    void *content = malloc(fileSize);
    // Read the file content into memory
    lseek(elf_fd, 0, SEEK_SET);
    ssize_t bytesread = read(elf_fd, content, fileSize);
    // Point the ELF header to the start of the file content
    ehdr1 = (Elf32_Ehdr *)content;
    // Point to the raw file data for further processing
    unsigned char *fileData = (unsigned char *)content;

    // Copy the ELF header from the file data to the ehdr1 variable
    memcpy(ehdr1, fileData, sizeof(Elf32_Ehdr));

    // Check if the ELF file and its format are valid
    if (!elf_check_file(ehdr1)) {
        exit(1);
    }
    // Check if the ELF file is supported
    if (!elf_check_supported(ehdr1)) {
        exit(1);
    }
    // Load and run the ELF executable using a custom function
    load_and_run_elf(argv);
    // Perform cleanup after executing the ELF file
    loader_cleanup();
    return 0;
}
