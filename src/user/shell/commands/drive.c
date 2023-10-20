#include <command.h>
#include <string.h>
#include <user/clock.h>
#include <user/drive.h>
#include <user/fd.h>
#include <user/io.h>
#include <user/memory.h>
#include <user/types.h>



// Must be page-aligned
#define SPEED_TEST_BUFFER_SIZE (32768*4096)

#define SPEED_TEST_COUNT 2



void drive_main(int argc,const char*const* argv){
	const char* drive_name=NULL;
	for (u32 i=1;i<argc;i++){
		if (argv[i][0]!='-'&&!drive_name){
			drive_name=argv[i];
		}
		else{
			printf("drive: unrecognized option '%s'\n",argv[i]);
			return;
		}
	}
	if (!drive_name){
		printf("drive: no drive supplied\n");
		return;
	}
	u32 i=0;
	drive_t drive;
	for (;drive_get(i,&drive);i++){
		if (string_equal(drive.name,drive_name)){
			goto _drive_found;
		}
	}
	printf("drive: drive '%s' not found\n",drive_name);
	return;
_drive_found:
	printf("Name: \x1b[1m%s\x1b[0m\nSize: \x1b[1m%v\x1b[0m\nBlock size: \x1b[1m%v\x1b[0m\nBlock count: \x1b[1m%lu\x1b[0m\n",
		drive_name,
		drive.block_count*drive.block_size,
		drive.block_size,
		drive.block_count
	);
}



DECLARE_COMMAND(drive,"drive <drive>");
