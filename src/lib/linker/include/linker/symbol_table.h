#ifndef _LINKER_SYMBOL_TABLE_H_
#define _LINKER_SYMBOL_TABLE_H_ 1
#include <core/types.h>




void symbol_table_add(const char* name,u64 address);




u64 symbol_table_lookup(const char* name);



#endif
