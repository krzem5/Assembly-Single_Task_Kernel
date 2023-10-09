#include <kernel/format/format.h>
#include <kernel/serial/serial.h>
#include <kernel/types.h>



#define BUFFER_SIZE 256



void log(const char* template,...){
	char buffer[BUFFER_SIZE];
	__builtin_va_list va;
	__builtin_va_start(va,template);
	u32 offset=format_string_va(buffer,BUFFER_SIZE,template,&va);
	__builtin_va_end(va);
	serial_send(buffer,offset);
}
