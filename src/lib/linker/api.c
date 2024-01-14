#include <linker/shared_object.h>
#include <linker/symbol.h>
#include <sys/types.h>



SYS_PUBLIC u64 __sys_linker_get_object_root(void){
	return (u64)shared_object_root;
}



SYS_PUBLIC u64 __sys_linker_get_object_next(u64 handle){
	return (u64)(((const shared_object_t*)handle)->next);
}



SYS_PUBLIC const char* __sys_linker_get_object_path(u64 handle){
	return ((const shared_object_t*)handle)->path;
}



SYS_PUBLIC u64 __sys_linker_get_object_base(u64 handle){
	return ((const shared_object_t*)handle)->image_base;
}

