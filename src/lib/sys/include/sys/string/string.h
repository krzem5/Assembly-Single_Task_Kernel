#ifndef _SYS_STRING_STRING_H_
#define _SYS_STRING_STRING_H_ 1
#include <sys/types.h>



u64 __attribute__((access(read_only,1),nonnull,warn_unused_result)) sys_string_length(const char* str);



s32 __attribute__((access(read_only,1),access(read_only,2),nonnull,warn_unused_result)) sys_string_compare(const char* a,const char* b);



s32 __attribute__((access(read_only,1,3),access(read_only,2,3),nonnull,warn_unused_result)) sys_string_compare_up_to(const char* a,const char* b,u64 length);



char* __attribute__((access(read_only,1),nonnull,warn_unused_result)) sys_string_duplicate(const char* str);



#endif
