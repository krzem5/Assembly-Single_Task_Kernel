#include <sys/error/error.h>
#include <sys/fd/fd.h>
#include <sys/io/io.h>
#include <sys/types.h>
#include <sys/util/options.h>



int main(int argc,const char** argv){
	const char* path=NULL;
	if (!sys_options_parse_NEW(argc,argv,"{:p:path}!s",&path)){
		return 1;
	}
	sys_fd_t fd=sys_fd_open(0,path,SYS_FD_FLAG_CREATE|SYS_FD_FLAG_DIRECTORY|SYS_FD_FLAG_EXCLUSIVE_CREATE);
	if (SYS_IS_ERROR(fd)){
		sys_io_print("mkdir: unable to create file '%s': error %d\n",path,fd);
		return 1;
	}
	sys_fd_close(fd);
	return 0;
}
