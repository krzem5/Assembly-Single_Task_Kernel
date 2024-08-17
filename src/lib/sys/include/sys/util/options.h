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
		bool* var_switch;
		const char** var_string;
		s64* var_int;
		bool (*var_string_callback)(const char*);
		bool (*var_int_callback)(s64);
	};
} sys_option_t;



/*
 * Syntax:
 * {<name_1a>:<name_1b>:...}<format_modifier_1><format_character_1>{<name_2a>:<name_2b>:...}<format_modifier_2><format_character_2>
 *
 * Format modifiers:
 * <empty>
 * * - accumulate options instead of overriding them
 * ! - required option
 * - - signed value
 * + - nonzero value
 *
 * Format characters:
 * a - u32 (aggregate options, end of argument processing)
 * b - bool (0/1/false/true)
 * i - s32
 * I - u32
 * q - s64
 * Q - u64
 * f - float
 * d - double
 * s - const char*
 * n - bool=0 (no following argument)
 * y - bool=1 (no following argument)
 */
bool __attribute__((access(read_only,2,1))) sys_options_parse_NEW(u32 argc,const char*const* argv,const char* template,...);



u32 __attribute__((access(read_only,2,1))) sys_options_parse(u32 argc,const char** argv,const sys_option_t* options);



s64 __attribute__((access(read_only,1))) sys_options_atoi(const char* str);



#endif
