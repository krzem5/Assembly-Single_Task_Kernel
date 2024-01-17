#include <sys/format/format.h>
#include <sys/time/time.h>
#include <sys/types.h>
#include <sys/util/var_arg.h>



#define FLAG_SHORT_SHORT 1
#define FLAG_SHORT 2
#define FLAG_LONG 4
#define FLAG_SIGN 8
#define FLAG_HEX 16
#define MASK_SIZE (FLAG_SHORT_SHORT|FLAG_SHORT|FLAG_LONG)



typedef struct _FORMAT_BUFFER_STATE{
	char* buffer;
	u32 offset;
	u32 length;
} format_buffer_state_t;



static inline void _buffer_state_add(format_buffer_state_t* buffer_state,char c){
	if (buffer_state->offset>=buffer_state->length){
		return;
	}
	buffer_state->buffer[buffer_state->offset]=c;
	buffer_state->offset++;
}



static inline char _format_base16_char(u8 value){
	value&=15;
	return value+(value>9?87:48);
}



static inline void _format_base10_int(u64 value,u32 min_width,format_buffer_state_t* out){
	char buffer[20];
	u8 i=0;
	do{
		buffer[i]=(value%10)+48;
		i++;
		value/=10;
	} while (value);
	for (;min_width>i;min_width--){
		_buffer_state_add(out,'0');
	}
	while (i){
		i--;
		_buffer_state_add(out,buffer[i]);
	}
}



static void _format_int(sys_var_arg_list_t* va,u8 flags,format_buffer_state_t* out){
	u64 data;
	if (flags&FLAG_SIGN){
		s64 signed_data=((flags&FLAG_LONG)?sys_var_arg_get(*va,s64):sys_var_arg_get(*va,s32));
		if (signed_data<0){
			_buffer_state_add(out,'-');
			signed_data=-signed_data;
		}
		data=signed_data;
	}
	else{
		data=((flags&FLAG_LONG)?sys_var_arg_get(*va,u64):sys_var_arg_get(*va,u32));
	}
	if (!data){
		_buffer_state_add(out,'0');
		return;
	}
	if (!(flags&FLAG_HEX)){
		_format_base10_int(data,0,out);
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



SYS_PUBLIC u32 sys_format_string(char* buffer,u32 length,const char* template,...){
	sys_var_arg_list_t va;
	sys_var_arg_init(va,template);
	u32 out=sys_format_string_va(buffer,length,template,&va);
	sys_var_arg_deinit(va);
	return out;
}



SYS_PUBLIC u32 sys_format_string_va(char* buffer,u32 length,const char* template,sys_var_arg_list_t* va){
	if (!length){
		return 0;
	}
	format_buffer_state_t out={
		buffer,
		0,
		length-1
	};
	while (*template&&out.offset<out.length){
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
		if (*template=='c'){
			_buffer_state_add(&out,sys_var_arg_get(*va,int));
		}
		else if (*template=='s'){
			const char* ptr=sys_var_arg_get(*va,const char*);
			if (!ptr){
				ptr="(null)";
			}
			while (*ptr){
				_buffer_state_add(&out,*ptr);
				ptr++;
			}
		}
		else if (*template=='f'){
			double value=sys_var_arg_get(*va,double);
			if (value<0){
				_buffer_state_add(&out,'-');
				value=-value;
			}
			_format_base10_int(value,0,&out);
			_buffer_state_add(&out,'.');
			_format_base10_int(((u64)(value*1000000))%1000000,6,&out);
		}
		else if (*template=='d'){
			_format_int(va,flags|FLAG_SIGN,&out);
		}
		else if (*template=='u'){
			_format_int(va,flags,&out);
		}
		else if (*template=='x'){
			_format_int(va,flags|FLAG_HEX,&out);
		}
		else if (*template=='X'){
			u8 value=(u8)sys_var_arg_get(*va,u32);
			_buffer_state_add(&out,_format_base16_char(value>>4));
			_buffer_state_add(&out,_format_base16_char(value));
		}
		else if (*template=='v'){
			u64 size=sys_var_arg_get(*va,u64);
			if (size<0x400){
				_format_base10_int(size,0,&out);
				_buffer_state_add(&out,' ');
				_buffer_state_add(&out,'B');
			}
			else if (size<0x100000){
				_format_base10_int((size+0x200)>>10,0,&out);
				_buffer_state_add(&out,' ');
				_buffer_state_add(&out,'K');
				_buffer_state_add(&out,'B');
			}
			else if (size<0x40000000){
				_format_base10_int((size+0x80000)>>20,0,&out);
				_buffer_state_add(&out,' ');
				_buffer_state_add(&out,'M');
				_buffer_state_add(&out,'B');
			}
			else if (size<0x10000000000ull){
				_format_base10_int((size+0x20000000)>>30,0,&out);
				_buffer_state_add(&out,' ');
				_buffer_state_add(&out,'G');
				_buffer_state_add(&out,'B');
			}
			else if (size<0x4000000000000ull){
				_format_base10_int((size+0x8000000000ull)>>40,0,&out);
				_buffer_state_add(&out,' ');
				_buffer_state_add(&out,'T');
				_buffer_state_add(&out,'B');
			}
			else if (size<0x1000000000000000ull){
				_format_base10_int((size+0x2000000000000ull)>>50,0,&out);
				_buffer_state_add(&out,' ');
				_buffer_state_add(&out,'P');
				_buffer_state_add(&out,'B');
			}
			else{
				_format_base10_int((size+0x800000000000000ull)>>60,0,&out);
				_buffer_state_add(&out,' ');
				_buffer_state_add(&out,'E');
				_buffer_state_add(&out,'B');
			}
		}
		else if (*template=='p'){
			u64 address=sys_var_arg_get(*va,u64);
			u32 shift=64;
			while (shift){
				if (shift==32){
					_buffer_state_add(&out,'_');
				}
				shift-=4;
				_buffer_state_add(&out,_format_base16_char(address>>shift));
			}
		}
		else if (*template=='g'){
			const u8* guid=sys_var_arg_get(*va,const u8*);
			for (u8 i=0;i<32;i++){
				if (i==9||i==13||i==17){
					_buffer_state_add(&out,'-');
				}
				_buffer_state_add(&out,_format_base16_char(guid[i>>1]>>((!(i&1))<<2)));
			}
		}
		else if (*template=='M'){
			const u8* mac_address=sys_var_arg_get(*va,const u8*);
			for (u8 i=0;i<12;i++){
				if (i&&!(i&1)){
					_buffer_state_add(&out,':');
				}
				_buffer_state_add(&out,_format_base16_char(mac_address[i>>1]>>((!(i&1))<<2)));
			}
		}
		else if (*template=='I'){
			u32 ip4=sys_var_arg_get(*va,u32);
			for (u8 i=0;i<4;i++){
				if (i){
					_buffer_state_add(&out,'.');
				}
				_format_base10_int((ip4>>(24-(i<<3)))&0xff,0,&out);
			}
		}
		else if (*template=='t'){
			s64 time=sys_var_arg_get(*va,s64);
			sys_time_t split_time;
			sys_time_from_nanoseconds(time,&split_time);
			_format_base10_int(split_time.years,4,&out);
			_buffer_state_add(&out,'-');
			_format_base10_int(split_time.months,2,&out);
			_buffer_state_add(&out,'-');
			_format_base10_int(split_time.days,2,&out);
			_buffer_state_add(&out,' ');
			_format_base10_int(split_time.hours,2,&out);
			_buffer_state_add(&out,':');
			_format_base10_int(split_time.minutes,2,&out);
			_buffer_state_add(&out,':');
			_format_base10_int(split_time.seconds,2,&out);
			_buffer_state_add(&out,'.');
			_format_base10_int(time%1000000000,9,&out);
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
