#ifndef _SYS_FORMAT_FORMAT_H_
#define _SYS_FORMAT_FORMAT_H_ 1
#include <sys/types.h>
#include <sys/util/var_arg.h>



u32 __attribute__((access(write_only,1,2))) sys_format_string(char* buffer,u32 length,const char* template,...);



u32 __attribute__((access(write_only,1,2),nonnull)) sys_format_string_va(char* buffer,u32 length,const char* template,sys_var_arg_list_t* va);



#endif
