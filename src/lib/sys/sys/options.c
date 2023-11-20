#include <sys/io.h>
#include <sys/options.h>
#include <sys/types.h>



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
		_Bool is_long_opt=0;
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
				printf("%s: unrecognized option '%s'\n",argv[0],opt-2);
			}
			else{
				printf("%s: invalid option -- '%c'\n",argv[0],opt[0]);
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
				printf("%s: missing argument for option '%s'\n",argv[0],argv[i-1]);
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
	_Bool negative=0;
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
