#include <sys/fd.h>
#include <sys/io.h>
#include <sys/options.h>



int main(int argc,const char** argv,const char** environ){
	s64 columns=16;
	s64 bytes=0;
	sys_option_t options[]={
		{
			.short_name='c',
			.long_name="columns",
			.var_type=SYS_OPTION_VAR_TYPE_INT,
			.flags=0,
			.var_int=&columns
		},
		{
			.short_name='b',
			.long_name="bytes",
			.var_type=SYS_OPTION_VAR_TYPE_INT,
			.flags=0,
			.var_int=&bytes
		},
		{
			.var_type=SYS_OPTION_VAR_TYPE_LAST
		}
	};
	if (!sys_options_parse(argc,argv,options)){
		return 1;
	}
	if (columns<=0){
		columns=1;
	}
	if (bytes<=0){
		bytes=1;
	}
	s64 fd=sys_fd_open(0,"/dev/random",SYS_FD_FLAG_READ);
	if (fd<0){
		printf("random: unable to open random file\n");
		return 1;
	}
	u8 buffer[512];
	u32 i=0;
	while (bytes){
		u64 count=(bytes>512?512:bytes);
		sys_fd_read(fd,buffer,count,0);
		bytes-=count;
		for (u32 j=0;j<count;j++){
			if (i>=columns){
				i=0;
				putchar('\n');
			}
			else if (i){
				putchar(' ');
			}
			i++;
			printf("%X",buffer[j]);
		}
	}
	putchar('\n');
	sys_fd_close(fd);
	return 0;
}
