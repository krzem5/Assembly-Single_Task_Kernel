#include <kernel/format/format.h>
#include <kernel/log/log.h>
#include <kernel/serial/serial.h>
#include <kernel/types.h>



#define BUFFER_SIZE 256



KERNEL_PUBLIC void log(u32 type,const char* name,const char* template,...){
	char buffer[BUFFER_SIZE];
	__builtin_va_list va;
	__builtin_va_start(va,template);
	u32 buffer_size=format_string_va(buffer,BUFFER_SIZE,template,&va);
	__builtin_va_end(va);
	(void)buffer_size;
	char final_buffer[BUFFER_SIZE];
	const char* format_code="0";
	if (type==LOG_TYPE_INFO){
		format_code="37";
	}
	else if (type==LOG_TYPE_LOG){
		format_code="1;97";
	}
	else if (type==LOG_TYPE_WARN){
		format_code="93";
	}
	else if (type==LOG_TYPE_ERROR){
		format_code="1;91";
	}
	serial_send(serial_default_port,final_buffer,format_string(final_buffer,BUFFER_SIZE,"\x1b[90m[%s] \x1b[%sm%s\x1b[0m\n",name,format_code,buffer));
}



KERNEL_PUBLIC void log_direct(const char* template,...){
	char buffer[BUFFER_SIZE];
	__builtin_va_list va;
	__builtin_va_start(va,template);
	u32 offset=format_string_va(buffer,BUFFER_SIZE,template,&va);
	__builtin_va_end(va);
	serial_send(serial_default_port,buffer,offset);
}
