#include <sys/fd/fd.h>
#include <sys/io/io.h>
#include <sys/types.h>
#include <sys/util/options.h>



int main(int argc,const char** argv){
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
	sys_fd_t fd=sys_fd_open(0,"/dev/random",SYS_FD_FLAG_READ);
	if (SYS_IS_ERROR(fd)){
		sys_io_print("random: unable to open random file\n");
		return 1;
	}
	u8 buffer[512];
	u32 i=0;
	while (bytes){
		u64 count=(bytes>512?512:bytes);
		count=sys_fd_read(fd,buffer,count,0);
		if (SYS_IS_ERROR(count)){
			sys_fd_close(fd);
			sys_io_print("random: unable to read from file: error %d\n",count);
			return 1;
		}
		bytes-=count;
		for (u32 j=0;j<count;j++){
			if (i>=columns){
				i=0;
				sys_io_print("\n");
			}
			else if (i){
				sys_io_print(" ");
			}
			i++;
			sys_io_print("%X",buffer[j]);
		}
	}
	sys_io_print("\n");
	sys_fd_close(fd);
	return 0;
}
