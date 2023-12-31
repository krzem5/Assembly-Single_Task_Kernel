#include <stdarg.h>
#include <sys/_kernel_syscall.h>
#include <sys/fd.h>
#include <sys/io.h>
#include <sys/time.h>
#include <sys/types.h>



#define PRINTF_BUFFER_SIZE 512

#define FLAG_SHORT_SHORT 1
#define FLAG_SHORT 2
#define FLAG_LONG 4
#define FLAG_SIGN 8
#define FLAG_HEX 16
#define MASK_SIZE (FLAG_SHORT_SHORT|FLAG_SHORT|FLAG_LONG)



typedef struct _BUFFER_STATE{
	char* buffer;
	u32 offset;
	u32 size;
} buffer_state_t;



static s64 _stdin_fd=-1;
static s64 _stdout_fd=-1;



static u32 _read_data_from_stdin(void* buffer,u32 size,_Bool blocking){
	if (_stdin_fd==-1){
		_stdin_fd=sys_fd_open(0,"/proc/self/stdin",SYS_FD_FLAG_READ);
	}
	return sys_fd_read(_stdin_fd,buffer,size,(blocking?0:SYS_FD_FLAG_NONBLOCKING));
}



static void _write_data_to_stdout(const void* buffer,u32 size){
	if (_stdout_fd==-1){
		_stdout_fd=sys_fd_open(0,"/proc/self/stdout",SYS_FD_FLAG_WRITE);
	}
	sys_fd_write(_stdout_fd,buffer,size,0);
}



static inline void _buffer_state_add(buffer_state_t* buffer_state,char c){
	if (buffer_state->offset>=buffer_state->size){
		return;
	}
	buffer_state->buffer[buffer_state->offset]=c;
	buffer_state->offset++;
}



static inline char _format_base16_char(u8 value){
	value&=15;
	return value+(value<10?48:87);
}



static inline void _print_int_base10(u64 value,buffer_state_t* out,u8 min_length){
	char buffer[20];
	u8 i=0;
	while (value){
		buffer[i]=(value%10)+48;
		i++;
		value/=10;
	}
	for (;min_length>i;min_length--){
		_buffer_state_add(out,'0');
	}
	while (i){
		i--;
		_buffer_state_add(out,buffer[i]);
	}
}



static void _print_int(va_list va,u8 flags,buffer_state_t* out){
	u64 data;
	if (flags&FLAG_SIGN){
		s64 signed_data=((flags&FLAG_LONG)?va_arg(va,s64):va_arg(va,s32));
		if (signed_data<0){
			_buffer_state_add(out,'-');
			signed_data=-signed_data;
		}
		data=signed_data;
	}
	else{
		data=((flags&FLAG_LONG)?va_arg(va,u64):va_arg(va,u32));
	}
	if (!data){
		_buffer_state_add(out,'0');
		return;
	}
	if (!(flags&FLAG_HEX)){
		_print_int_base10(data,out,0);
		return;
	}
	char buffer[16];
	u8 i=0;
	while (data){
		buffer[i]=_format_base16_char(data);
		i++;
		data>>=4;
	}
	while (i){
		i--;
		_buffer_state_add(out,buffer[i]);
	}
}



SYS_PUBLIC void printf(const char* template,...){
	char buffer[PRINTF_BUFFER_SIZE];
	va_list va;
	va_start(va,template);
	_write_data_to_stdout(buffer,svprintf(buffer,PRINTF_BUFFER_SIZE,template,va));
	va_end(va);
}



u32 sprintf(char* buffer,u32 size,const char* template,...){
	va_list va;
	va_start(va,template);
	u32 out=svprintf(buffer,size,template,va);
	va_end(va);
	return out;
}



u32 svprintf(char* buffer,u32 size,const char* template,va_list va){
	buffer_state_t out={
		buffer,
		0,
		size
	};
	while (*template&&out.offset<out.size){
		if (*template!='%'){
			_buffer_state_add(&out,*template);
			template++;
			continue;
		}
		u8 flags=0;
		while (1){
			template++;
			if (!*template){
				return out.offset;
			}
			if (*template=='l'){
				flags=(flags&(~MASK_SIZE))|FLAG_LONG;
			}
			else if (*template=='h'&&*(template+1)=='h'){
				flags=(flags&(~MASK_SIZE))|FLAG_SHORT_SHORT;
				template++;
			}
			else if (*template=='h'){
				flags=(flags&(~MASK_SIZE))|FLAG_SHORT;
			}
			else{
				break;
			}
		}
		if (*template=='c'){
			_buffer_state_add(&out,va_arg(va,int));
		}
		else if (*template=='s'){
			const char* ptr=va_arg(va,const char*);
			if (!ptr){
				ptr="(null)";
			}
			while (*ptr){
				_buffer_state_add(&out,*ptr);
				ptr++;
			}
		}
		else if (*template=='d'){
			_print_int(va,flags|FLAG_SIGN,&out);
		}
		else if (*template=='u'){
			_print_int(va,flags,&out);
		}
		else if (*template=='x'){
			_print_int(va,flags|FLAG_HEX,&out);
		}
		else if (*template=='X'){
			u32 value=va_arg(va,u32);
			_buffer_state_add(&out,_format_base16_char(value>>4));
			_buffer_state_add(&out,_format_base16_char(value));
		}
		else if (*template=='v'){
			u64 size=va_arg(va,u64);
			if (!size){
				_buffer_state_add(&out,'0');
				_buffer_state_add(&out,' ');
				_buffer_state_add(&out,'B');
			}
			else if (size<0x400){
				_print_int_base10(size,&out,0);
				_buffer_state_add(&out,' ');
				_buffer_state_add(&out,'B');
			}
			else if (size<0x100000){
				_print_int_base10((size+0x200)>>10,&out,0);
				_buffer_state_add(&out,' ');
				_buffer_state_add(&out,'K');
				_buffer_state_add(&out,'B');
			}
			else if (size<0x40000000){
				_print_int_base10((size+0x80000)>>20,&out,0);
				_buffer_state_add(&out,' ');
				_buffer_state_add(&out,'M');
				_buffer_state_add(&out,'B');
			}
			else if (size<0x10000000000ull){
				_print_int_base10((size+0x20000000)>>30,&out,0);
				_buffer_state_add(&out,' ');
				_buffer_state_add(&out,'G');
				_buffer_state_add(&out,'B');
			}
			else if (size<0x4000000000000ull){
				_print_int_base10((size+0x8000000000ull)>>40,&out,0);
				_buffer_state_add(&out,' ');
				_buffer_state_add(&out,'T');
				_buffer_state_add(&out,'B');
			}
			else if (size<0x1000000000000000ull){
				_print_int_base10((size+0x2000000000000ull)>>50,&out,0);
				_buffer_state_add(&out,' ');
				_buffer_state_add(&out,'P');
				_buffer_state_add(&out,'B');
			}
			else{
				_print_int_base10((size+0x800000000000000ull)>>60,&out,0);
				_buffer_state_add(&out,' ');
				_buffer_state_add(&out,'E');
				_buffer_state_add(&out,'B');
			}
		}
		else if (*template=='p'){
			u64 address=va_arg(va,u64);
			u32 shift=64;
			while (shift){
				if (shift==32){
					_buffer_state_add(&out,'_');
				}
				shift-=4;
				_buffer_state_add(&out,_format_base16_char(address>>shift));
			}
		}
		else if (*template=='t'){
			s64 time=va_arg(va,s64);
			sys_time_t split_time;
			sys_time_from_nanoseconds(time,&split_time);
			_print_int_base10(split_time.years,&out,4);
			_buffer_state_add(&out,'-');
			_print_int_base10(split_time.months,&out,2);
			_buffer_state_add(&out,'-');
			_print_int_base10(split_time.days,&out,2);
			_buffer_state_add(&out,' ');
			_print_int_base10(split_time.hours,&out,2);
			_buffer_state_add(&out,':');
			_print_int_base10(split_time.minutes,&out,2);
			_buffer_state_add(&out,':');
			_print_int_base10(split_time.seconds,&out,2);
			_buffer_state_add(&out,'.');
			_print_int_base10(time%1000000000,&out,9);
		}
		else{
			_buffer_state_add(&out,*template);
		}
		template++;
	}
	return out.offset;
}



SYS_PUBLIC void print_buffer(const void* buffer,u32 length){
	_write_data_to_stdout(buffer,length);
}



SYS_PUBLIC void putchar(char c){
	_write_data_to_stdout(&c,1);
}



SYS_PUBLIC char getchar(void){
	char out;
	_read_data_from_stdin(&out,1,1);
	return out;
}



SYS_PUBLIC int getchar_timeout(u64 timeout){
	char out;
	return (_read_data_from_stdin(&out,1,0)==1?out:-1);
}
