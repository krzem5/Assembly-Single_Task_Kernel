#include <sys/heap/heap.h>
#include <sys/io/io.h>
#include <sys/memory/memory.h>
#include <sys/types.h>
#include <sys/util/options.h>
#include <sys/util/var_arg.h>



#define OPTION_DATA_FLAG_ACCUMULATE 0x01
#define OPTION_DATA_FLAG_REQUIRED 0x02
#define OPTION_DATA_FLAG_SIGNED 0x04
#define OPTION_DATA_FLAG_NONZERO 0x08



typedef struct _OPTION_GROUP{
	struct _OPTION_GROUP* next;
	bool seen;
	char name[];
} option_group_t;



typedef struct _OPTION_DATA{
	struct _OPTION_DATA* next;
	option_group_t* group;
	char type;
	u32 flags;
	u32 name_length;
	union{
		void* ptr;
		bool* data_bool;
		u32* data_u32;
		u64* data_u64;
		float* data_float;
		double* data_double;
		const char** data_str;
		bool** data_bool_array;
		u32** data_u32_array;
		u64** data_u64_array;
		float** data_float_array;
		double** data_double_array;
		const char*** data_str_array;
	};
	u32* data_length;
	char name[];
} option_data_t;



#define OPTION_EMIT_DATA(type,value) \
	if (option->flags&OPTION_DATA_FLAG_ACCUMULATE){ \
		(*(option->data_length))++; \
		*(option->data_##type##_array)=sys_heap_realloc(NULL,*(option->data_##type##_array),(*(option->data_length))*sizeof(*(option->data_##type))); \
		(*(option->data_##type##_array))[*(option->data_length)-1]=value; \
	} \
	else{ \
		*(option->data_##type)=value; \
	}



static bool _parse_option(option_data_t* option,const char* exe_name,const char* option_name,const char* arg){
	if (!option){
		sys_io_print("%s: unknown option '%s'\n",exe_name,(option_name?option_name:arg));
		return 0;
	}
	option->group->seen=1;
	if (option->type=='s'){
		OPTION_EMIT_DATA(str,arg);
		return 1;
	}
	if (option->type=='n'){
		*(option->data_bool)=0;
		return 1;
	}
	if (option->type=='y'){
		*(option->data_bool)=1;
		return 1;
	}
	if (option->type=='i'||option->type=='q'){
		const char* base=arg;
		bool negative=0;
		if (arg[0]=='-'||arg[0]=='+'){
			negative=(arg[0]=='-');
			arg++;
		}
		if (!arg[0]){
			sys_io_print("%s: '%s' is not a valid integer\n",exe_name,base);
			return 0;
		}
		u64 mask=(option->type=='i'?0xffffffff:0xffffffffffffffffull);
		u64 value=0;
		do{
			if (arg[0]<'0'||arg[0]>'9'){
				sys_io_print("%s: invalid base-10 character: '%c'\n",exe_name,arg[0]);
				return 0;
			}
			u64 tmp=(value*10+arg[0]-'0')&mask;
			if (tmp<value){
				sys_io_print("%s: integer overflow: '%s'\n",exe_name,base);
				return 0;
			}
			value=tmp;
			arg++;
		} while (arg[0]);
		if (negative&&(value>>(option->type=='i'?31:63))){
			sys_io_print("%s: integer underflow: '%s'\n",exe_name,base);
			return 0;
		}
		if (!(option->flags&OPTION_DATA_FLAG_SIGNED)&&negative){
			sys_io_print("%s: option '%s' requires an unsigned integer\n",exe_name,option->group->name);
			return 0;
		}
		if (!(option->flags&OPTION_DATA_FLAG_NONZERO)&&!value){
			sys_io_print("%s: option '%s' requires a nonzero integer\n",exe_name,option->group->name);
			return 0;
		}
		if (option->type=='i'){
			OPTION_EMIT_DATA(u32,(negative?-value:value));
		}
		else{
			OPTION_EMIT_DATA(u64,(negative?-value:value));
		}
		return 1;
	}
	sys_io_print("<UNIMPLEMENTED:'%s':'%s','%c',0x%x>\n",arg,option->name,option->type,option->flags);
	return 0;
}



SYS_PUBLIC bool sys_options_parse_NEW(u32 argc,const char*const* argv,const char* template,...){
	if (!argc){
		return 0;
	}
	option_group_t* group_head=NULL;
	option_group_t* group_tail=NULL;
	option_data_t* unnamed=NULL;
	option_data_t* short_head=NULL;
	option_data_t* short_tail=NULL;
	option_data_t* long_head=NULL;
	option_data_t* long_tail=NULL;
	bool out=0;
	sys_var_arg_list_t va_list;
	sys_var_arg_init(va_list,template);
	for (;template[0];template++){
		if (template[0]!='{'){
			goto _error;
		}
		template++;
		const char* ptr=template;
		for (;template[0]&&template[0]!='}';template++);
		if (!template[0]){
			goto _error;
		}
		u32 option_name_length=template-ptr;
		template++;
		u32 flags=0;
		for (;template[0]=='*'||template[0]=='!'||template[0]=='-'||template[0]=='+';template++){
			if (template[0]=='*'){
				flags|=OPTION_DATA_FLAG_ACCUMULATE;
			}
			else if (template[0]=='!'){
				flags|=OPTION_DATA_FLAG_REQUIRED;
			}
			else if (template[0]=='-'){
				flags|=OPTION_DATA_FLAG_SIGNED;
			}
			else if (template[0]=='+'){
				flags|=OPTION_DATA_FLAG_NONZERO;
			}
		}
		if (template[0]!='a'&&template[0]!='b'&&template[0]!='i'&&template[0]!='q'&&template[0]!='f'&&template[0]!='d'&&template[0]!='s'&&template[0]!='n'&&template[0]!='y'){
			goto _error;
		}
		if ((flags&(OPTION_DATA_FLAG_SIGNED|OPTION_DATA_FLAG_NONZERO))&&template[0]!='i'&&template[0]!='q'&&template[0]!='f'&&template[0]!='d'){
			goto _error;
		}
		if ((flags&OPTION_DATA_FLAG_ACCUMULATE)&&template[0]=='a'){
			goto _error;
		}
		void* out_ptr=sys_var_arg_get(va_list,void*);
		u32* out_ptr_length=((flags&OPTION_DATA_FLAG_ACCUMULATE)?sys_var_arg_get(va_list,u32*):NULL);
		option_group_t* group=sys_heap_alloc(NULL,sizeof(option_group_t)+option_name_length+1);
		group->next=NULL;
		group->seen=!(flags&OPTION_DATA_FLAG_REQUIRED);
		sys_memory_copy(ptr,group->name,option_name_length);
		group->name[option_name_length]=0;
		if (group_tail){
			group_tail->next=group;
		}
		else{
			group_head=group;
		}
		group_tail=group;
		while (1){
			u32 i=0;
			for (;ptr[i]!='}'&&ptr[i]!=':';i++);
			if (!i){
				if (unnamed){
					goto _error;
				}
				unnamed=sys_heap_alloc(NULL,sizeof(option_data_t)+1);
				unnamed->group=group;
				unnamed->type=template[0];
				unnamed->flags=flags;
				unnamed->name_length=0;
				unnamed->ptr=out_ptr;
				unnamed->data_length=out_ptr_length;
				unnamed->name[0]=0;
				goto _skip_option_name;
			}
			if (i==1&&ptr[0]=='-'){
				ptr++;
				i=0;
			}
			option_data_t* option=sys_heap_alloc(NULL,sizeof(option_data_t)+i+1);
			option->next=NULL;
			option->group=group;
			option->type=template[0];
			option->flags=flags;
			option->name_length=i;
			option->ptr=out_ptr;
			option->data_length=out_ptr_length;
			sys_memory_copy(ptr,option->name,i);
			option->name[i]=0;
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
_skip_option_name:
			ptr+=i;
			if (ptr[0]==':'){
				ptr++;
				continue;
			}
			ptr++;
			break;
		}
	}
	sys_var_arg_deinit(va_list);
	for (u32 i=1;i<argc;i++){
		if (argv[i][0]!='-'){
			if (!_parse_option(unnamed,argv[0],NULL,argv[i])){
				goto _cleanup;
			}
			continue;
		}
		u32 j=0;
		for (;argv[i][j]&&argv[i][j]!='=';j++);
		u32 k=(argv[i][1]=='-')+1;
		option_data_t* option=(k==2?long_head:short_head);
		for (;option&&(option->name_length!=j-k||sys_memory_compare(argv[i]+k,option->name,j-k));option=option->next);
		if (option&&(option->type=='a'||option->type=='n'||option->type=='y')){
			if (argv[i][j]=='='){
				sys_io_print("%s: unexpected argument for option '%s'\n",argv[0],option->group->name);
				goto _cleanup;
			}
			if (option->type=='a'){
				if (i+1==argc){
					sys_io_print("%s: missing argument for option '%s'\n",argv[0],option->group->name);
					goto _cleanup;
				}
				option->group->seen=1;
				*(option->data_u32)=i+1;
				break;
			}
			if (!_parse_option(option,argv[0],argv[i],NULL)){
				goto _cleanup;
			}
			continue;
		}
		if (argv[i][j]!='='&&i+1==argc){
			sys_io_print("%s: missing argument for option '%s'\n",argv[0],option->group->name);
			goto _cleanup;
		}
		if (!_parse_option(option,argv[0],argv[i],(argv[i][j]=='='?argv[i]+j+1:argv[i+1]))){
			goto _cleanup;
		}
		i+=(argv[i][j]!='=');
	}
	out=1;
_cleanup:
	for (option_group_t* group=group_head;group;){
		if (out&&!group->seen){
			sys_io_print("%s: missing option '%s'\n",argv[0],group->name);
			out=0;
		}
		option_group_t* next=group->next;
		sys_heap_dealloc(NULL,group);
		group=next;
	}
	sys_heap_dealloc(NULL,unnamed);
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
