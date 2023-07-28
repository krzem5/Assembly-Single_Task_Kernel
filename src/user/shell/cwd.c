#include <user/fs.h>
#include <user/io.h>
#include <user/types.h>



char cwd[4096];
u32 cwd_length;
int cwd_fd;



void cwd_init(void){
	cwd[0]='/';
	cwd[1]=0;
	cwd_length=1;
	cwd_fd=fs_open(0,"/",0);
}



_Bool cwd_change(const char* path){
	int new_cwd_fd=fs_open(cwd_fd,path,0);
	if (new_cwd_fd<0){
		return 0;
	}
	fs_close(cwd_fd);
	cwd_fd=new_cwd_fd;
	return 1;
}
