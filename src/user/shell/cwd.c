#include <sys/fd.h>
#include <sys/io.h>
#include <sys/types.h>



char cwd[4096];
u32 cwd_length;
s64 cwd_fd;



void cwd_init(void){
	cwd[0]='/';
	cwd[1]=0;
	cwd_length=1;
	cwd_fd=sys_fd_open(0,"/",0);
}



_Bool cwd_change(const char* path){
	s64 new_cwd_fd=sys_fd_open(cwd_fd,path,0);
	if (new_cwd_fd<0){
		return 0;
	}
	sys_fd_close(cwd_fd);
	cwd_fd=new_cwd_fd;
	cwd_length=sys_fd_path(cwd_fd,cwd,4096);
	return 1;
}
