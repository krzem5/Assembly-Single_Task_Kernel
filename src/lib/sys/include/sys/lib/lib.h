#ifndef _SYS_LIB_LIB_H_
#define _SYS_LIB_LIB_H_ 1
#include <sys/error/error.h>
#include <sys/types.h>



typedef u64 sys_library_t;



sys_library_t sys_lib_get_root(void);



sys_library_t sys_lib_get_next(sys_library_t library);



const char* sys_lib_get_path(sys_library_t library);



u64 sys_lib_get_image_base(sys_library_t library);



u64 sys_lib_lookup_symbol(sys_library_t library,const char* name);



#endif
