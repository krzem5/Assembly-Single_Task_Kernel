#ifndef _LINKER_SYMBOL_H_
#define _LINKER_SYMBOL_H_ 1
#include <linker/shared_object.h>
#include <sys/types.h>



u64 linker_symbol_lookup_by_name(const char* name);



u64 linker_symbol_lookup_by_name_in_shared_object(const linker_shared_object_t* so,const char* name);



u64 linker_symbol_resolve_plt(const linker_shared_object_t* so,u64 index);



void linker_symbol_resolve_plt_trampoline(void);



#endif
