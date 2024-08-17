#ifndef _SYS_UTIL_OPTIONS_H_
#define _SYS_UTIL_OPTIONS_H_ 1
#include <sys/types.h>



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
bool __attribute__((access(read_only,2,1))) sys_options_parse(u32 argc,const char*const* argv,const char* template,...);\



#endif
