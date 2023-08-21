#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

typedef struct {
    uint8_t e_ident[16];
    uint16_t e_type;
    uint16_t e_machine;
    uint32_t e_version;
    uint32_t e_entry;
    uint32_t e_phoff;
    uint32_t e_shoff;
    uint32_t e_flags;
    uint16_t e_ehsize;
    uint16_t e_phentsize;
    uint16_t e_phnum;
    uint16_t e_shentsize;
    uint16_t e_shnum;
    uint16_t e_shstrndx;
} Elf32_Ehdr;

int main() {
    FILE *elfFile = fopen("fib", "rb");
    if (!elfFile) {
        perror("Failed to open ELF file");
        return 1;
    }

    Elf32_Ehdr elfHeader;
    fread(&elfHeader, sizeof(elfHeader), 1, elfFile);

    // Check if the file is an ELF file
    if (elfHeader.e_ident[0] != 0x7F || elfHeader.e_ident[1] != 'E' ||
        elfHeader.e_ident[2] != 'L' || elfHeader.e_ident[3] != 'F') {
        printf("Not an ELF file\n");
        fclose(elfFile);
        return 1;
    }

    // Now you can access different fields in elfHeader to extract information
    printf("Entry point: 0x%x\n", elfHeader.e_phnum);

    fclose(elfFile);
    return 0;
}
