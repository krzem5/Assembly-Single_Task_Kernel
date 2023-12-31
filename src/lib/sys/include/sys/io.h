#ifndef _SYS_IO_H_
#define _SYS_IO_H_ 1
#include <stdarg.h>
#include <sys/types.h>



void printf(const char* template,...);



u32 sprintf(char* buffer,u32 size,const char* template,...);



u32 svprintf(char* buffer,u32 size,const char* template,va_list va);



void print_buffer(const void* buffer,u32 length);



void putchar(char c);



char getchar(void);



int getchar_timeout(u64 timeout);



#endif
