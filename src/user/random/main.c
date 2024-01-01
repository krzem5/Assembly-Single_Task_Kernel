#include <sys2/fd/fd.h>
#include <sys2/io/io.h>
#include <sys2/types.h>
#include <sys2/util/options.h>



int main(int argc,const char** argv){
	s64 columns=16;
	s64 bytes=0;
	sys2_option_t options[]={
		{
			.short_name='c',
			.long_name="columns",
			.var_type=SYS2_OPTION_VAR_TYPE_INT,
			.flags=0,
			.var_int=&columns
		},
		{
			.short_name='b',
			.long_name="bytes",
			.var_type=SYS2_OPTION_VAR_TYPE_INT,
			.flags=0,
			.var_int=&bytes
		},
		{
			.var_type=SYS2_OPTION_VAR_TYPE_LAST
		}
	};
	if (!sys2_options_parse(argc,argv,options)){
		return 1;
	}
	if (columns<=0){
		columns=1;
	}
	if (bytes<=0){
		bytes=1;
	}
	sys2_fd_t fd=sys2_fd_open(0,"/dev/random",SYS2_FD_FLAG_READ);
	if (SYS2_IS_ERROR(fd)){
		sys2_io_print("random: unable to open random file\n");
		return 1;
	}
	u8 buffer[512];
	u32 i=0;
	while (bytes){
		u64 count=(bytes>512?512:bytes);
		count=sys2_fd_read(fd,buffer,count,0);
		if (SYS2_IS_ERROR(count)){
			sys2_fd_close(fd);
			sys2_io_print("random: unable to read from file: error %d\n",count);
			return 1;
		}
		bytes-=count;
		for (u32 j=0;j<count;j++){
			if (i>=columns){
				i=0;
				sys2_io_print("\n");
			}
			else if (i){
				sys2_io_print(" ");
			}
			i++;
			sys2_io_print("%X",buffer[j]);
		}
	}
	sys2_io_print("\n");
	sys2_fd_close(fd);
	return 0;
}
