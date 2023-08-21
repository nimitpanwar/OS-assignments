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
    print_memory_content(content, fileSize);

    return 0;
}
