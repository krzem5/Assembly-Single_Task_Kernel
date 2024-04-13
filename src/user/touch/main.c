#include <sys/error/error.h>
#include <sys/fd/fd.h>
#include <sys/io/io.h>
#include <sys/types.h>
#include <sys/util/options.h>



int main(int argc,const char** argv){
	u32 i=sys_options_parse(argc,argv,NULL);
	if (!i||i>=argc){
		return 1;
	}
	sys_fd_t fd=sys_fd_open(0,argv[i],SYS_FD_FLAG_CREATE);
	if (SYS_IS_ERROR(fd)){
		sys_io_print("touch: unable to open file '%s': error %d\n",argv[i],fd);
		return 1;
	}
	sys_fd_close(fd);
	return 0;
}
