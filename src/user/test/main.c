#include <sys/io.h>
#include <sys/syscall.h>



#define OPTION_VAR_TYPE_LAST 0
#define OPTION_VAR_TYPE_NONE 1
#define OPTION_VAR_TYPE_SWITCH 2
#define OPTION_VAR_TYPE_STRING 3
#define OPTION_VAR_TYPE_INT 4

#define OPTION_FLAG_CALLBACK 1
#define OPTION_FLAG_CLEAR_FLAG 2



typedef struct _OPTION{
	char short_name;
	const char* long_name;
	u8 var_type;
	u8 flags;
	union{
		_Bool* var_switch;
		const char** var_string;
		s64* var_int;
		_Bool (*var_string_callback)(const char*);
		_Bool (*var_int_callback)(s64);
	};
} option_t;



u32 parse_options(u32 argc,const char** argv,const option_t* options){
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
		const option_t* option=NULL;
		_Bool is_long_opt=0;
		if (opt[0]=='-'){
			is_long_opt=1;
			opt++;
			for (const option_t* option=options;option->var_type!=OPTION_VAR_TYPE_LAST;option++){
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
			for (const option_t* option=options;option->var_type!=OPTION_VAR_TYPE_LAST;option++){
				if (option->short_name==opt[0]){
					opt++;
					break;
				}
			}
		}
		if (!option){
			if (is_long_opt){
				printf("%s: unrecognized option '%s'\n",argv[0],opt-2);
			}
			else{
				printf("%s: invalid option -- '%c'\n",argv[0],opt[0]);
			}
			return 0;
		}
		if (option->var_type==OPTION_VAR_TYPE_NONE){
			continue;
		}
		if (option->var_type==OPTION_VAR_TYPE_SWITCH){
			*(option->var_switch)=!(option->flags&OPTION_FLAG_CLEAR_FLAG);
			continue;
		}
		const char* arg=NULL;
		if (!opt[0]){
			if (i==first_arg_index){
				printf("%s: missing argument for option '%s'\n",argv[0],argv[i-1]);
				return 0;
			}
			arg=argv[i];
			i++;
		}
		else{
			arg=opt+1;
		}
		if (option->var_type!=OPTION_VAR_TYPE_INT){
			if (option->flags&OPTION_FLAG_CALLBACK){
				if (!option->var_string_callback(arg)){
					return 0;
				}
			}
			else{
				*(option->var_string)=arg;
			}
			continue;
		}
		_Bool negative=0;
		u32 j=0;
		for (;arg[j]=='+'||arg[j]=='-';j++){
			negative=(arg[j]=='-');
		}
		s64 value=0;
		for (;arg[j];j++){
			if (arg[j]<48||arg[j]>57){
				printf("%s: '%s' is not an integer\n",argv[0],arg);
				return 0;
			}
			value=value*10+arg[0]-48;
		}
		if (negative){
			value=-value;
		}
		if (option->flags&OPTION_FLAG_CALLBACK){
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



void main(int argc,const char** argv,const char** environ){
	printf("\nargc=%u\n",argc);
	for (int i=0;i<argc;i++){
		printf("argv[%u]=\"%s\"\n",i,argv[i]);
	}
	for (int i=0;environ[i];i++){
		printf("environ[%u]=\"%s\"\n",i,environ[i]);
	}
}
