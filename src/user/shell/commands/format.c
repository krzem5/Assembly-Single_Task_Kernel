#include <command.h>
#include <string.h>
#include <user/drive.h>
#include <user/fs.h>
#include <user/io.h>
#include <user/memory.h>
#include <user/types.h>



#define BOOT_CODE_FILE_NAME "/core.bin"



void format_main(int argc,const char*const* argv){
	_Bool boot=0;
	const char* drive_name=NULL;
	for (u32 i=1;i<argc;i++){
		if (string_equal(argv[i],"-b")){
			boot=1;
		}
		else if (argv[i][0]!='-'&&!drive_name){
			drive_name=argv[i];
		}
		else{
			printf("format: unrecognized option '%s'\n",argv[i]);
			return;
		}
	}
	if (!drive_name){
		printf("format: no drive supplied\n");
		return;
	}
	u32 i=0;
	for (;i<drive_count;i++){
		if (string_equal((drives+i)->name,drive_name)){
			break;
		}
	}
	if (i==drive_count){
		printf("format: drive '%s' not found\n",drive_name);
		return;
	}
	u8* boot_code=NULL;
	u32 boot_code_length=0;
	if (boot){
		int fd=fs_open(0,BOOT_CODE_FILE_NAME,FS_FLAG_READ);
		if (fd<0){
			printf("format: unable to open file '%s': error %d\n",BOOT_CODE_FILE_NAME,fd);
			return;
		}
		boot_code_length=fs_seek(fd,0,FS_SEEK_END);
		fs_seek(fd,0,FS_SEEK_SET);
		boot_code=memory_map(boot_code_length,0);
		fs_read(fd,boot_code,boot_code_length);
		fs_close(fd);
	}
	if (!drive_format(i,boot_code,boot_code_length)){
		printf("format: failed to format drive\n");
	}
	else{
		printf("format: drive formatted successfully, reboot required\n");
	}
	if (boot){
		memory_unmap(boot_code,boot_code_length);
	}
}



DECLARE_COMMAND(format,"format <drive>");
