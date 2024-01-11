#ifndef _SYS_UTIL_OPTIONS_H_
#define _SYS_UTIL_OPTIONS_H_ 1
#include <sys/types.h>



#define SYS_OPTION_VAR_TYPE_LAST 0
#define SYS_OPTION_VAR_TYPE_NONE 1
#define SYS_OPTION_VAR_TYPE_SWITCH 2
#define SYS_OPTION_VAR_TYPE_STRING 3
#define SYS_OPTION_VAR_TYPE_INT 4

#define SYS_OPTION_FLAG_CALLBACK 1
#define SYS_OPTION_FLAG_CLEAR_FLAG 2



typedef struct _SYS_OPTION{
	char short_name;
	const char* long_name;
	u8 var_type;
	u8 flags;
	union{
		_Bool* var_switch;
		const char** var_string;
		s64* var_int;
		_Bool (*var_string_callback)(const char*);
		_Bool (*var_int_callback)(s64);
	};
} sys_option_t;



u32 __attribute__((access(read_only,2,1),nonnull)) sys_options_parse(u32 argc,const char** argv,const sys_option_t* options);



s64 __attribute__((access(read_only,1))) sys_options_atoi(const char* str);



#endif
