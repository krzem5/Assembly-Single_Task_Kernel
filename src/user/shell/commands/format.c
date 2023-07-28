#include <command.h>
#include <string.h>
#include <user/drive.h>
#include <user/io.h>
#include <user/types.h>



void format_main(int argc,const char*const* argv){
	_Bool boot=0;
	const char* drive_name=NULL;
	for (u32 i=1;i<argc;i++){
		if (i<argc-1&&string_equal(argv[i],"-b")){
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
	(void)boot;
	if (!drive_format(i,NULL,0)){
		printf("format: failed to format drive\n");
	}
	else{
		printf("format: drive formatted successfully, reboot required\n");
	}
}



DECLARE_COMMAND(format,"format <drive>");
