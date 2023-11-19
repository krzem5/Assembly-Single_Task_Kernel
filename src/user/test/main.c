#include <sys/io.h>
#include <sys/options.h>
#include <sys/syscall.h>



void main(int argc,const char** argv,const char** environ){
	s64 columns=16;
	sys_option_t options[]={
		{
			.short_name='c',
			.long_name="columns",
			.var_type=SYS_OPTION_VAR_TYPE_INT,
			.flags=0,
			.var_int=&columns
		},
		{
			.var_type=SYS_OPTION_VAR_TYPE_LAST
		}
	};
	u32 first_arg_index=sys_options_parse(argc,argv,options);
	printf("\nfirst_arg_index: %u, columns: %ld\n",first_arg_index,columns);
	printf("argc=%u\n",argc);
	for (int i=0;i<argc;i++){
		printf("argv[%u]=\"%s\"\n",i,argv[i]);
	}
	for (int i=0;environ[i];i++){
		printf("environ[%u]=\"%s\"\n",i,environ[i]);
	}
}
