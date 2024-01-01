#ifndef _LINKER_SYMBOL_H_
#define _LINKER_SYMBOL_H_ 1
#include <linker/shared_object.h>
#include <sys2/types.h>



u64 symbol_lookup_by_name(const char* name);



u64 symbol_resolve_plt(shared_object_t* so,u64 index);



void symbol_resolve_plt_trampoline(void);



#endif
