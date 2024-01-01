#ifndef _SYS2_FORMAT_FORMAT_H_
#define _SYS2_FORMAT_FORMAT_H_ 1
#include <sys2/types.h>
#include <sys2/util/var_arg.h>



u32 sys2_format_string(char* buffer,u32 length,const char* template,...);



u32 sys2_format_string_va(char* buffer,u32 length,const char* template,sys2_var_arg_list_t* va);



#endif
