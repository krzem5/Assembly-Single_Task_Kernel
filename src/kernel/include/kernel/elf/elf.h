#ifndef _KERNEL_ELF_ELF_H_
#define _KERNEL_ELF_ELF_H_ 1
#include <kernel/handle/handle.h>
#include <kernel/types.h>



#define ELF_AUXV_PLATFORM "x86_64"
#define ELF_AUXV_RANDOM_DATA_SIZE 16

#define ELF_LOAD_FLAG_PAUSE_THREAD 1



handle_id_t elf_load(const char* path,u32 argc,const char*const* argv,const char*const* environ,u32 flags);



#endif
