#ifndef _SYS_UTIL_VAR_ARG_H_
#define _SYS_UTIL_VAR_ARG_H_ 1



#define sys_var_arg_init(va,after) __builtin_va_start(va,after)
#define sys_var_arg_deinit(va) __builtin_va_end(va)
#define sys_var_arg_get(va,type) __builtin_va_arg(va,type)



typedef __builtin_va_list sys_var_arg_list_t;



#endif
