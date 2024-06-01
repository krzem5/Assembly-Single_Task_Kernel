#ifndef _KERNEL_ELF_ELF_H_
#define _KERNEL_ELF_ELF_H_ 1
#include <kernel/error/error.h>
#include <kernel/types.h>



#define ELF_AUXV_PLATFORM "x86_64"
#define ELF_AUXV_RANDOM_DATA_SIZE 16

#define ELF_LOAD_FLAG_PAUSE_THREAD 1
#define ELF_LOAD_FLAG_DEFAULT_IO 2



error_t elf_load(const char* path,u32 argc,const char*const* argv,u32 environ_length,const char*const* environ,u32 flags);



u32 elf_get_hwcap(void);



#endif
