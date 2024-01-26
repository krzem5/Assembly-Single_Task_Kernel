#include <sys/error/error.h>
#include <sys/lib/lib.h>
#include <sys/types.h>



extern u64 __sys_linker_get_object_root(void) __attribute__((weak));
extern u64 __sys_linker_get_object_next(u64 handle) __attribute__((weak));
extern const char* __sys_linker_get_object_path(u64 handle) __attribute__((weak));
extern u64 __sys_linker_get_object_base(u64 handle) __attribute__((weak));
extern u64 __sys_linker_lookup_symbol(u64 handle,const char* name) __attribute__((weak));
extern u64 __sys_linker_load_library(const char* name,u32 flags) __attribute__((weak));
extern const char* __sys_linker_update_library_path(const char* new) __attribute__((weak));



SYS_PUBLIC sys_library_t sys_lib_get_root(void){
	return __sys_linker_get_object_root();
}



SYS_PUBLIC sys_library_t sys_lib_get_next(sys_library_t library){
	return __sys_linker_get_object_next(library);
}



SYS_PUBLIC const char* sys_lib_get_path(sys_library_t library){
	return __sys_linker_get_object_path(library);
}



SYS_PUBLIC u64 sys_lib_get_image_base(sys_library_t library){
	return __sys_linker_get_object_base(library);
}



SYS_PUBLIC sys_library_t sys_lib_load(const char* name,u32 flags){
	return __sys_linker_load_library(name,flags);
}



SYS_PUBLIC u64 sys_lib_lookup_symbol(sys_library_t library,const char* name){
	return __sys_linker_lookup_symbol(library,name);
}



SYS_PUBLIC const char* sys_lib_get_search_path(void){
	return __sys_linker_update_library_path(NULL);
}



SYS_PUBLIC void sys_lib_set_search_path(const char* path){
	__sys_linker_update_library_path(path);
}
