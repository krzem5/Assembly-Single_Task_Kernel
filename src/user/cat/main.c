#include <sys/error/error.h>
#include <sys/fd/fd.h>
#include <sys/io/io.h>
#include <sys/types.h>
#include <sys/util/options.h>



int main(int argc,const char** argv){
	u32 i=sys_options_parse(argc,argv,NULL);
	if (!i){
		return 1;
	}
	for (;i<argc;i++){
		sys_fd_t fd=sys_fd_open(0,(argv[i][0]=='-'&&!argv[i][1]?"/proc/self/stdin":argv[i]),SYS_FD_FLAG_READ);
		if (SYS_IS_ERROR(fd)){
			sys_io_print("cat: unable to open file '%s': error %d\n",argv[i],fd);
			return 1;
		}
		char buffer[512];
		while (1){
			u64 length=sys_fd_read(fd,buffer,512,0);
			if (SYS_IS_ERROR(length)){
				sys_io_print("cat: unable to read from file '%s': error %d\n",argv[i],length);
				sys_fd_close(fd);
				return 1;
			}
			if (!length){
				break;
			}
			sys_fd_write(sys_io_output_fd,buffer,length,0);
		}
		sys_fd_close(fd);
		sys_io_print("\n");
	}
	return 0;
}
