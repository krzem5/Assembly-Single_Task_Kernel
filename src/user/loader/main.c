#include <user/elf.h>
#include <user/fs.h>
#include <user/types.h>



void main(void){
	int fd=fs_open(0,"/kernel/startup.txt",FS_FLAG_READ);
	if (fd<0){
		goto _error;
	}
	char buffer[4096];
	u32 length=fs_read(fd,buffer,4096);
	fs_close(fd);
	if (length<=0){
		goto _error;
	}
	while (length&&(buffer[length-1]=='\n'||buffer[length-1]=='\r'||buffer[length-1]=='\t'||buffer[length-1]==' ')){
		length--;
	}
	buffer[length]=0;
	elf_load(buffer);
_error:
	elf_load("/kernel/shell.elf");
}
