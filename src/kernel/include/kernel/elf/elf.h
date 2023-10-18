#ifndef _KERNEL_ELF_ELF_H_
#define _KERNEL_ELF_ELF_H_ 1
#include <kernel/vfs2/node.h>



_Bool elf_load(const char* path);



_Bool elf_load2(vfs2_node_t* node);



#endif
