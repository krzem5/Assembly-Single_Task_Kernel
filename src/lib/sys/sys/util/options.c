#include <sys/heap/heap.h>
#include <sys/io/io.h>
#include <sys/memory/memory.h>
#include <sys/types.h>
#include <sys/util/options.h>
#include <sys/util/var_arg.h>



#define OPTION_DATA_FLAG_ACCUMULATE 0x01
#define OPTION_DATA_FLAG_REQUIRED 0x02



typedef struct _OPTION_DATA{
	struct _OPTION_DATA* next;
	char type;
	u32 flags;
	u32 name_length;
	union{
		void* ptr;
		bool* data_bool;
		s32* data_s32;
		u32* data_u32;
		s64* data_s64;
		u64* data_u64;
		double* data_double;
		const char** data_str;
	};
	char name[];
} option_data_t;



/*
 * Syntax:
 * {<name_1a>:<name_1b>:...}<format_modifier_1><format_character_1>{<name_2a>:<name_2b>:...}<format_modifier_2><format_character_2>
 *
 * Format modifiers:
 * <empty>
 * * - accumulate options instead of overriding them
 * ! - required option
 *
 * Format characters:
 * a - u32 (aggregate options, end of argument processing)
 * b - bool (0/1/false/true)
 * i - s32
 * I - u32
 * q - s64
 * Q - u64
 * f - double
 * s - const char*
 * n - bool=0 (no following argument)
 * y - bool=1 (no following argument)
 */
SYS_PUBLIC bool sys_options_parse_NEW(u32 argc,const char*const* argv,const char* template,...){
	if (!argc){
		return 0;
	}
	option_data_t* short_head=NULL;
	option_data_t* short_tail=NULL;
	option_data_t* long_head=NULL;
	option_data_t* long_tail=NULL;
	bool out=0;
	sys_var_arg_list_t va_list;
	sys_var_arg_init(va_list,template);
	for (;template[0];template++){
		if (template[0]!='{'||template[1]=='}'){
			goto _error;
		}
		template++;
		const char* ptr=template;
		for (;template[0]&&template[0]!='}';template++);
		if (!template[0]){
			goto _error;
		}
		u32 flags=0;
		for (;template[0]=='*'||template[0]=='!';template++){
			if (template[0]=='*'){
				flags|=OPTION_DATA_FLAG_ACCUMULATE;
			}
			else if (template[0]=='!'){
				flags|=OPTION_DATA_FLAG_REQUIRED;
			}
		}
		if (template[0]=='a'&&template[0]=='b'&&template[0]=='i'&&template[0]=='I'&&template[0]=='q'&&template[0]=='Q'&&template[0]=='f'&&template[0]=='s'&&template[0]=='n'&&template[0]=='y'){
			goto _error;
		}
		void* out_ptr=sys_var_arg_get(va_list,void*);
		while (1){
			u32 i=0;
			for (;ptr[i]&&ptr[i]!='}'&&ptr[i]!=':';i++);
			if (!i||!ptr[i]){
				goto _error;
			}
			if (i==1&&ptr[0]=='-'){
				ptr++;
				i=0;
			}
			option_data_t* option=sys_heap_alloc(NULL,sizeof(option_data_t)+i);
			option->next=NULL;
			option->type=template[0];
			option->flags=flags;
			option->name_length=i;
			option->ptr=out_ptr;
			sys_memory_copy(ptr,option->name,i);
			if (i==1){
				if (short_tail){
					short_tail->next=option;
				}
				else{
					short_head=option;
				}
				short_tail=option;
			}
			else{
				if (long_tail){
					long_tail->next=option;
				}
				else{
					long_head=option;
				}
				long_tail=option;
			}
			ptr+=i;
			if (ptr[0]==':'){
				ptr++;
				continue;
			}
			if (ptr[0]=='}'){
				ptr++;
				break;
			}
			goto _error;
		}
	}
	sys_var_arg_deinit(va_list);

	out=1;
_cleanup:
	for (option_data_t* option=short_head;option;){
		option_data_t* next=option->next;
		sys_heap_dealloc(NULL,option);
		option=next;
	}
	for (option_data_t* option=long_head;option;){
		option_data_t* next=option->next;
		sys_heap_dealloc(NULL,option);
		option=next;
	}
	return out;
_error:
	sys_io_print("%s: option template string error\n",argv[0]);
	goto _cleanup;
}



SYS_PUBLIC u32 sys_options_parse(u32 argc,const char** argv,const sys_option_t* options){
	u32 first_arg_index=argc;
	for (u32 i=1;i<first_arg_index;){
		const char* opt=argv[i];
		if (opt[0]!='-'||!opt[1]){
			for (u32 j=i;j<argc-1;j++){
				argv[j]=argv[j+1];
			}
			argv[argc-1]=opt;
			first_arg_index--;
			continue;
		}
		i++;
		opt++;
		const sys_option_t* option=NULL;
		bool is_long_opt=0;
		if (opt[0]=='-'){
			is_long_opt=1;
			opt++;
			for (option=options;option&&option->var_type!=SYS_OPTION_VAR_TYPE_LAST;option++){
				if (!option->long_name){
					continue;
				}
				u32 j=0;
				while (1){
					if (opt[j]!=option->long_name[j]){
						goto _check_next_option;
					}
					j++;
					if (!opt[j]||opt[j]=='='){
						break;
					}
				}
				if (!option->long_name[j]){
					opt+=j;
					break;
				}
_check_next_option:
			}
		}
		else{
			for (option=options;option&&option->var_type!=SYS_OPTION_VAR_TYPE_LAST;option++){
				if (option->short_name==opt[0]){
					opt++;
					break;
				}
			}
		}
		if (!option||option->var_type==SYS_OPTION_VAR_TYPE_LAST){
			if (is_long_opt){
				sys_io_print("%s: unrecognized option '%s'\n",argv[0],opt-2);
			}
			else{
				sys_io_print("%s: invalid option -- '%c'\n",argv[0],opt[0]);
			}
			return 0;
		}
		if (option->var_type==SYS_OPTION_VAR_TYPE_NONE){
			continue;
		}
		if (option->var_type==SYS_OPTION_VAR_TYPE_SWITCH){
			*(option->var_switch)=!(option->flags&SYS_OPTION_FLAG_CLEAR_FLAG);
			continue;
		}
		const char* arg=NULL;
		if (!opt[0]){
			if (i==first_arg_index){
				sys_io_print("%s: missing argument for option '%s'\n",argv[0],argv[i-1]);
				return 0;
			}
			arg=argv[i];
			i++;
		}
		else{
			arg=opt+is_long_opt;
		}
		if (option->var_type!=SYS_OPTION_VAR_TYPE_INT){
			if (option->flags&SYS_OPTION_FLAG_CALLBACK){
				if (!option->var_string_callback(arg)){
					return 0;
				}
			}
			else{
				*(option->var_string)=arg;
			}
			continue;
		}
		s64 value=sys_options_atoi(arg);
		if (option->flags&SYS_OPTION_FLAG_CALLBACK){
			if (!option->var_int_callback(value)){
				return 0;
			}
		}
		else{
			*(option->var_int)=value;
		}
	}
	return first_arg_index;
}



SYS_PUBLIC s64 sys_options_atoi(const char* str){
	bool negative=0;
	u32 i=0;
	for (;str[i]=='+'||str[i]=='-';i++){
		negative=(str[i]=='-');
	}
	s64 value=0;
	for (;str[i];i++){
		if (str[i]<48||str[i]>57){
			return 0;
		}
		value=value*10+str[i]-48;
	}
	return (negative?-value:value);
}
