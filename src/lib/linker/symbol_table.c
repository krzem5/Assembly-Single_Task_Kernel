#include <core/io.h>
#include <core/types.h>




void symbol_table_add(const char* name,u64 address){
	printf("%p %s\n",address,name);
}




u64 symbol_table_lookup(const char* name);
