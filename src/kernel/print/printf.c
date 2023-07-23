#include <kernel/lock/lock.h>
#include <kernel/print/print.h>
#include <kernel/types.h>



#define FLAG_SHORT_SHORT 1
#define FLAG_SHORT 2
#define FLAG_LONG 4
#define FLAG_SIGN 8
#define FLAG_HEX 16
#define MASK_SIZE (FLAG_SHORT_SHORT|FLAG_SHORT|FLAG_LONG)



static lock_t _stdio_lock=LOCK_INIT_STRUCT;



static inline char _format_base16_char(u8 value){
	value&=15;
	return value+(value<10?48:87);
}



static inline void _print_int_base10(u64 value){
	char buffer[20];
	u8 i=0;
	while (value){
		buffer[i]=(value%10)+48;
		i++;
		value/=10;
	}
	while (i){
		i--;
		_putchar_nolock(buffer[i]);
	}
}



static void _print_int(__builtin_va_list va,u8 flags){
	u64 data;
	if (flags&FLAG_SIGN){
		s64 signed_data=((flags&FLAG_LONG)?__builtin_va_arg(va,s64):__builtin_va_arg(va,s32));
		if (signed_data<0){
			_putchar_nolock('-');
			signed_data=-signed_data;
		}
		data=signed_data;
	}
	else{
		data=((flags&FLAG_LONG)?__builtin_va_arg(va,u64):__builtin_va_arg(va,u32));
	}
	if (!data){
		_putchar_nolock('0');
		return;
	}
	if (!(flags&FLAG_HEX)){
		_print_int_base10(data);
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
		_putchar_nolock(buffer[i]);
	}
}



void print_format(const char* template,...){
	lock_acquire(&_stdio_lock);
	__builtin_va_list va;
	__builtin_va_start(va,template);
	while (*template){
		if (*template!='%'){
			_putchar_nolock(*template);
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
			_putchar_nolock(__builtin_va_arg(va,int));
		}
		else if (*template=='s'){
			const char* ptr=__builtin_va_arg(va,const char*);
			if (!ptr){
				ptr="(null)";
			}
			while (*ptr){
				_putchar_nolock(*ptr);
				ptr++;
			}
		}
		else if (*template=='d'){
			_print_int(va,flags|FLAG_SIGN);
		}
		else if (*template=='u'){
			_print_int(va,flags);
		}
		else if (*template=='x'){
			_print_int(va,flags|FLAG_HEX);
		}
		else if (*template=='v'){
			u64 size=__builtin_va_arg(va,u64);
			if (!size){
				_putchar_nolock('0');
				_putchar_nolock(' ');
				_putchar_nolock('B');
			}
			else if (size<0x400){
				_print_int_base10(size);
				_putchar_nolock(' ');
				_putchar_nolock('B');
			}
			else if (size<0x100000){
				_print_int_base10((size+0x200)>>10);
				_putchar_nolock(' ');
				_putchar_nolock('K');
				_putchar_nolock('B');
			}
			else if (size<0x40000000){
				_print_int_base10((size+0x80000)>>20);
				_putchar_nolock(' ');
				_putchar_nolock('M');
				_putchar_nolock('B');
			}
			else if (size<0x10000000000ull){
				_print_int_base10((size+0x20000000)>>30);
				_putchar_nolock(' ');
				_putchar_nolock('G');
				_putchar_nolock('B');
			}
			else if (size<0x4000000000000ull){
				_print_int_base10((size+0x8000000000ull)>>40);
				_putchar_nolock(' ');
				_putchar_nolock('T');
				_putchar_nolock('B');
			}
			else if (size<0x1000000000000000ull){
				_print_int_base10((size+0x2000000000000ull)>>50);
				_putchar_nolock(' ');
				_putchar_nolock('P');
				_putchar_nolock('B');
			}
			else{
				_print_int_base10((size+0x800000000000000ull)>>60);
				_putchar_nolock(' ');
				_putchar_nolock('E');
				_putchar_nolock('B');
			}
		}
		else if (*template=='p'){
			u64 address=__builtin_va_arg(va,u64);
			u32 shift=64;
			while (shift){
				if (shift==32){
					_putchar_nolock('_');
				}
				shift-=4;
				_putchar_nolock(_format_base16_char(address>>shift));
			}
		}
		else{
			_putchar_nolock(*template);
		}
		template++;
	}
	__builtin_va_end(va);
	lock_release(&_stdio_lock);
}



void print_string(const char* str,u64 length){
	lock_acquire(&_stdio_lock);
	while (length){
		_putchar_nolock(*str);
		str++;
		length--;
	}
	lock_release(&_stdio_lock);
}
