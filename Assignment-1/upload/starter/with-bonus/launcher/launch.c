#include "/home/samyak/upload/starter/with-bonus/loader/loader.h"

Elf32_Ehdr *ehdr;
Elf32_Phdr *phdr;
int fd;

// int elf_check_file(Elf32_Ehdr *hdr) {
// 	if(!hdr) return 0;
// 	if(hdr->e_ident[EI_MAG0] != ELFMAG0) {
// 		printf("ELF Header EI_MAG0 incorrect.\n");
// 		return 0;
// 	}
// 	if(hdr->e_ident[EI_MAG1] != ELFMAG1) {
// 		printf("ELF Header EI_MAG1 incorrect.\n");
// 		return 0;
// 	}
// 	if(hdr->e_ident[EI_MAG2] != ELFMAG2) {
// 		printf("ELF Header EI_MAG2 incorrect.\n");
// 		return 0;
// 	}
// 	if(hdr->e_ident[EI_MAG3] != ELFMAG3) {
// 		printf("ELF Header EI_MAG3 incorrect.\n");
// 		return 0;
// 	}
// 	return 1;
// }

// int elf_check_supported(Elf32_Ehdr *hdr) {
// 	if(!elf_check_file(hdr)) {
// 		printf("Invalid ELF File.\n");
// 		return 0;
// 	}
// 	if(hdr->e_ident[EI_CLASS] != ELFCLASS32) {
// 		printf("Unsupported ELF File Class.\n");
// 		return 0;
// 	}
// 	if(hdr->e_ident[EI_DATA] != ELFDATA2LSB) {
// 		printf("Unsupported ELF File byte order.\n");
// 		return 0;
// 	}
// 	if(hdr->e_machine != EM_386) {
// 		printf("Unsupported ELF File target.\n");
// 		return 0;
// 	}
// 	if(hdr->e_ident[EI_VERSION] != EV_CURRENT) {
// 		printf("Unsupported ELF File version.\n");
// 		return 0;
// 	}
// 	if(hdr->e_type != ET_REL && hdr->e_type != ET_EXEC) {
// 		printf("Unsupported ELF File type.\n");
// 		return 0;
// 	}
// 	return 1;
// }


int main(int argc, char** argv) 
{
  if(argc != 2) {
    printf("Usage: %s <ELF Executable> \n",argv[0]);
    exit(1);
  }

//   fd = open(argv[1], O_RDONLY);
//   off_t fileSize = lseek(fd, 0, SEEK_END);
//   void *content = malloc(fileSize);
//   lseek(fd, 0, SEEK_SET);
//   ssize_t bytesread= read(fd, content, fileSize);
//   // print_memory_content(content, fileSize);
//   ehdr = (Elf32_Ehdr *)content;
//   unsigned char *fileData = (unsigned char *)content;
//   memcpy(ehdr,fileData,sizeof(Elf32_Ehdr));

//   if(!elf_check_file(ehdr)){
//     exit(1);
//   }

//   if(!elf_check_supported(ehdr)){
//     exit(1);
//   }

  // 1. carry out necessary checks on the input ELF file
  // 2. passing it to the loader for carrying out the loading/execution
  // setenv("LD_LIBRARY_PATH", "../loader", 1);
  load_and_run_elf(argv);
  // 3. invoke the cleanup routine inside the loader  
  loader_cleanup();
  return 0;
}
