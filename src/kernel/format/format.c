#include <kernel/format/format.h>
#include <kernel/types.h>



#define FLAG_SHORT_SHORT 1
#define FLAG_SHORT 2
#define FLAG_LONG 4
#define FLAG_SIGN 8
#define FLAG_HEX 16
#define MASK_SIZE (FLAG_SHORT_SHORT|FLAG_SHORT|FLAG_LONG)

#define LOWEST_FORMAT 'c'
#define HIGHEST_FORMAT 'x'



typedef struct _FORMAT_BUFFER_STATE{
	char* buffer;
	u32 offset;
	u32 length;
} format_buffer_state_t;



typedef void (*format_format_t)(__builtin_va_list*,u8,format_buffer_state_t*);



static KERNEL_CORE_RDATA const char _format_null_str[]="(null)";
static KERNEL_CORE_RDATA const char _format_base16_chars[]="0123456789abcdef";



static KERNEL_INLINE void KERNEL_CORE_CODE _buffer_state_add(format_buffer_state_t* buffer_state,char c){
	buffer_state->buffer[buffer_state->offset]=c;
	buffer_state->offset+=(buffer_state->offset<buffer_state->length);
}



static KERNEL_INLINE char KERNEL_CORE_CODE _format_base16_char(u8 value){
	return _format_base16_chars[value&15];
}



static KERNEL_INLINE void KERNEL_CORE_CODE _format_int_base10(u64 value,format_buffer_state_t* out){
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



static void KERNEL_CORE_CODE _format_int(__builtin_va_list* va,u8 flags,format_buffer_state_t* out){
	u64 data;
	if (flags&FLAG_SIGN){
		s64 signed_data=((flags&FLAG_LONG)?__builtin_va_arg(*va,s64):__builtin_va_arg(*va,s32));
		if (signed_data<0){
			_buffer_state_add(out,'-');
			signed_data=-signed_data;
		}
		data=signed_data;
	}
	else{
		data=((flags&FLAG_LONG)?__builtin_va_arg(*va,u64):__builtin_va_arg(*va,u32));
	}
	if (!data){
		_buffer_state_add(out,'0');
		return;
	}
	if (!(flags&FLAG_HEX)){
		_format_int_base10(data,out);
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



static void KERNEL_CORE_CODE _format_format_char(__builtin_va_list* va,u8 flags,format_buffer_state_t* out){
	_buffer_state_add(out,__builtin_va_arg(*va,int));
}



static void KERNEL_CORE_CODE _format_format_string(__builtin_va_list* va,u8 flags,format_buffer_state_t* out){
	const char* ptr=__builtin_va_arg(*va,const char*);
	if (!ptr){
		ptr=_format_null_str;
	}
	while (*ptr){
		_buffer_state_add(out,*ptr);
		ptr++;
	}
}



static void KERNEL_CORE_CODE _format_format_decimal(__builtin_va_list* va,u8 flags,format_buffer_state_t* out){
	_format_int(va,flags|FLAG_SIGN,out);
}



static void KERNEL_CORE_CODE _format_format_unsigned(__builtin_va_list* va,u8 flags,format_buffer_state_t* out){
	_format_int(va,flags,out);
}



static void KERNEL_CORE_CODE _format_format_hexadecimal(__builtin_va_list* va,u8 flags,format_buffer_state_t* out){
	_format_int(va,flags|FLAG_HEX,out);
}



static void KERNEL_CORE_CODE _format_format_volume(__builtin_va_list* va,u8 flags,format_buffer_state_t* out){
	u64 size=__builtin_va_arg(*va,u64);
	if (!size){
		_buffer_state_add(out,'0');
		_buffer_state_add(out,' ');
		_buffer_state_add(out,'B');
	}
	else if (size<0x400){
		_format_int_base10(size,out);
		_buffer_state_add(out,' ');
		_buffer_state_add(out,'B');
	}
	else if (size<0x100000){
		_format_int_base10((size+0x200)>>10,out);
		_buffer_state_add(out,' ');
		_buffer_state_add(out,'K');
		_buffer_state_add(out,'B');
	}
	else if (size<0x40000000){
		_format_int_base10((size+0x80000)>>20,out);
		_buffer_state_add(out,' ');
		_buffer_state_add(out,'M');
		_buffer_state_add(out,'B');
	}
	else if (size<0x10000000000ull){
		_format_int_base10((size+0x20000000)>>30,out);
		_buffer_state_add(out,' ');
		_buffer_state_add(out,'G');
		_buffer_state_add(out,'B');
	}
	else if (size<0x4000000000000ull){
		_format_int_base10((size+0x8000000000ull)>>40,out);
		_buffer_state_add(out,' ');
		_buffer_state_add(out,'T');
		_buffer_state_add(out,'B');
	}
	else if (size<0x1000000000000000ull){
		_format_int_base10((size+0x2000000000000ull)>>50,out);
		_buffer_state_add(out,' ');
		_buffer_state_add(out,'P');
		_buffer_state_add(out,'B');
	}
	else{
		_format_int_base10((size+0x800000000000000ull)>>60,out);
		_buffer_state_add(out,' ');
		_buffer_state_add(out,'E');
		_buffer_state_add(out,'B');
	}
}



static void KERNEL_CORE_CODE _format_format_pointer(__builtin_va_list* va,u8 flags,format_buffer_state_t* out){
	u64 address=__builtin_va_arg(*va,u64);
	u32 shift=64;
	while (shift){
		if (shift==32){
			_buffer_state_add(out,'_');
		}
		shift-=4;
		_buffer_state_add(out,_format_base16_char(address>>shift));
	}
}



static KERNEL_CORE_RDATA const format_format_t _format_formats[HIGHEST_FORMAT-LOWEST_FORMAT+1]={
	['c'-LOWEST_FORMAT]=_format_format_char,
	['s'-LOWEST_FORMAT]=_format_format_string,
	['d'-LOWEST_FORMAT]=_format_format_decimal,
	['u'-LOWEST_FORMAT]=_format_format_unsigned,
	['x'-LOWEST_FORMAT]=_format_format_hexadecimal,
	['v'-LOWEST_FORMAT]=_format_format_volume,
	['p'-LOWEST_FORMAT]=_format_format_pointer
};



u32 KERNEL_CORE_CODE format_string(char* buffer,u32 length,const char* template,...){
	__builtin_va_list va;
	__builtin_va_start(va,template);
	u32 out=format_string_va(buffer,length,template,&va);
	__builtin_va_end(va);
	return out;
}



u32 KERNEL_CORE_CODE format_string_va(char* buffer,u32 length,const char* template,__builtin_va_list* va){
	if (!length){
		return 0;
	}
	format_buffer_state_t out={
		buffer,
		0,
		length-1
	};
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
				goto _end;
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
		if (*template>=LOWEST_FORMAT&&*template<=HIGHEST_FORMAT&&_format_formats[(u8)(*template)-LOWEST_FORMAT]){
			_format_formats[(u8)(*template)-LOWEST_FORMAT](va,flags,&out);
		}
		else{
			_buffer_state_add(&out,*template);
		}
		template++;
	}
_end:
	out.buffer[out.offset]=0;
	return out.offset;
}
