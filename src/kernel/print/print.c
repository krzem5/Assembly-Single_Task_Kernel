#include <kernel/print/print.h>
#include <kernel/serial/serial.h>
#include <kernel/types.h>



#define BUFFER_SIZE 256

#define FLAG_SHORT_SHORT 1
#define FLAG_SHORT 2
#define FLAG_LONG 4
#define FLAG_SIGN 8
#define FLAG_HEX 16
#define MASK_SIZE (FLAG_SHORT_SHORT|FLAG_SHORT|FLAG_LONG)



typedef struct _BUFFER_STATE{
	char buffer[BUFFER_SIZE];
	u32 offset;
} buffer_state_t;



static inline void KERNEL_CORE_CODE _buffer_state_add(buffer_state_t* buffer_state,char c){
	buffer_state->buffer[buffer_state->offset]=c;
	if (buffer_state->offset<BUFFER_SIZE){
		buffer_state->offset++;
	}
}



static inline char KERNEL_CORE_CODE _format_base16_char(u8 value){
	value&=15;
	return value+(value<10?48:87);
}



static inline void KERNEL_CORE_CODE _print_int_base10(u64 value,buffer_state_t* out){
	char buffer[20];
	u8 i=0;
	while (value){
		buffer[i]=(value%10)+48;
		i++;
		value/=10;
	}
	while (i){
		i--;
		_buffer_state_add(out,buffer[i]);
	}
}



static void KERNEL_CORE_CODE _print_int(__builtin_va_list va,u8 flags,buffer_state_t* out){
	u64 data;
	if (flags&FLAG_SIGN){
		s64 signed_data=((flags&FLAG_LONG)?__builtin_va_arg(va,s64):__builtin_va_arg(va,s32));
		if (signed_data<0){
			_buffer_state_add(out,'-');
			signed_data=-signed_data;
		}
		data=signed_data;
	}
	else{
		data=((flags&FLAG_LONG)?__builtin_va_arg(va,u64):__builtin_va_arg(va,u32));
	}
	if (!data){
		_buffer_state_add(out,'0');
		return;
	}
	if (!(flags&FLAG_HEX)){
		_print_int_base10(data,out);
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



void KERNEL_CORE_CODE print(const char* template,...){
	buffer_state_t out={
		.offset=0
	};
	__builtin_va_list va;
	__builtin_va_start(va,template);
	while (*template){
		if (*template!='%'){
			_buffer_state_add(&out,*template);
			template++;
			continue;
		}
		u8 flags=0;
		while (1){
			template++;
			if (!*template){
				return;
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
			_buffer_state_add(&out,__builtin_va_arg(va,int));
		}
		else if (*template=='s'){
			const char* ptr=__builtin_va_arg(va,const char*);
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
		else if (*template=='v'){
			u64 size=__builtin_va_arg(va,u64);
			if (!size){
				_buffer_state_add(&out,'0');
				_buffer_state_add(&out,' ');
				_buffer_state_add(&out,'B');
			}
			else if (size<0x400){
				_print_int_base10(size,&out);
				_buffer_state_add(&out,' ');
				_buffer_state_add(&out,'B');
			}
			else if (size<0x100000){
				_print_int_base10((size+0x200)>>10,&out);
				_buffer_state_add(&out,' ');
				_buffer_state_add(&out,'K');
				_buffer_state_add(&out,'B');
			}
			else if (size<0x40000000){
				_print_int_base10((size+0x80000)>>20,&out);
				_buffer_state_add(&out,' ');
				_buffer_state_add(&out,'M');
				_buffer_state_add(&out,'B');
			}
			else if (size<0x10000000000ull){
				_print_int_base10((size+0x20000000)>>30,&out);
				_buffer_state_add(&out,' ');
				_buffer_state_add(&out,'G');
				_buffer_state_add(&out,'B');
			}
			else if (size<0x4000000000000ull){
				_print_int_base10((size+0x8000000000ull)>>40,&out);
				_buffer_state_add(&out,' ');
				_buffer_state_add(&out,'T');
				_buffer_state_add(&out,'B');
			}
			else if (size<0x1000000000000000ull){
				_print_int_base10((size+0x2000000000000ull)>>50,&out);
				_buffer_state_add(&out,' ');
				_buffer_state_add(&out,'P');
				_buffer_state_add(&out,'B');
			}
			else{
				_print_int_base10((size+0x800000000000000ull)>>60,&out);
				_buffer_state_add(&out,' ');
				_buffer_state_add(&out,'E');
				_buffer_state_add(&out,'B');
			}
		}
		else if (*template=='p'){
			u64 address=__builtin_va_arg(va,u64);
			u32 shift=64;
			while (shift){
				if (shift==32){
					_buffer_state_add(&out,'_');
				}
				shift-=4;
				_buffer_state_add(&out,_format_base16_char(address>>shift));
			}
		}
		else{
			_buffer_state_add(&out,*template);
		}
		template++;
	}
	__builtin_va_end(va);
	serial_send(out.buffer,out.offset);
}
