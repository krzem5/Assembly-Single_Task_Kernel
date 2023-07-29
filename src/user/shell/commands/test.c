#include <command.h>
#include <cwd.h>
#include <user/fs.h>



void test_main(int argc,const char*const* argv){
	cwd_change("ahci0p0:/");
	int fd=fs_open(0,"ahci0p0:a",FS_FLAG_WRITE|FS_FLAG_CREATE);
	fs_write(fd,"Hello world!\n",13);
	fs_close(fd);
}



DECLARE_COMMAND(test,"test");
