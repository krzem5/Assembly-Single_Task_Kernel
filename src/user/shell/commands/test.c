#include <command.h>
#include <cwd.h>
#include <user/fs.h>



void test_main(int argc,const char*const* argv){
	cwd_change("ahci0p0:/");
	int fd=fs_open(0,"ahci0p0:a",FS_FLAG_WRITE|FS_FLAG_CREATE);
	fs_write(fd,"Hello $$$",9);
	fs_seek(fd,6,FS_SEEK_SET);
	fs_write(fd,"world!\n",7);
	fs_close(fd);
}



DECLARE_COMMAND(test,"test");
