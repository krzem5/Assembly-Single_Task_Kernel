#include <command.h>
#include <string.h>
#include <user/drive.h>
#include <user/io.h>
#include <user/types.h>



void drive_main(int argc,const char*const* argv){
	if (argc<2){
		printf("drive: no drive supplied\n");
		return;
	}
	if (argc>2){
		printf("drive: unrecognized option '%s'\n",argv[2]);
		return;
	}
	u32 i=0;
	for (;i<drive_count;i++){
		if (string_equal((drives+i)->name,argv[1])){
			break;
		}
	}
	if (i==drive_count){
		printf("drive: drive '%s' not found\n",argv[1]);
		return;
	}
	drive_stats_t stats;
	if (!drive_get_stats(i,&stats)){
		printf("drive: unable to get drive stats\n");
		return;
	}
}



DECLARE_COMMAND(drive,"drive <drive>");
