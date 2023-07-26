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
	cwd_fd=fs_open("/",0);
}



void cwd_change(const char* path){
	printf("Unable to change directory to '%s'\n",path);
}
