#ifndef _KERNEL_ELF_ELF_H_
#define _KERNEL_ELF_ELF_H_ 1
#include <kernel/types.h>



#define ELF_AUXV_PLATFORM "x86_64"
#define ELF_AUXV_RANDOM_DATA_SIZE 16



_Bool elf_load(const char* path,u32 argc,const char*const* argv,const char*const* environ);



#endif
