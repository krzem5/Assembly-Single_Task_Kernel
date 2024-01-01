#include <sys2/fd/fd.h>
#include <sys2/types.h>



char cwd[4096];
u32 cwd_length;
sys2_fd_t cwd_fd;



void cwd_init(void){
	cwd[0]='/';
	cwd[1]=0;
	cwd_length=1;
	cwd_fd=sys2_fd_open(0,"/",0);
}



_Bool cwd_change(const char* path){
	sys2_fd_t new_cwd_fd=sys2_fd_open(cwd_fd,path,0);
	if (SYS2_IS_ERROR(new_cwd_fd)){
		return 0;
	}
	sys2_fd_close(cwd_fd);
	cwd_fd=new_cwd_fd;
	cwd_length=sys2_fd_path(cwd_fd,cwd,4096);
	return 1;
}
