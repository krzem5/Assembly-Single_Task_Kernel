#include <command.h>
#include <string.h>
#include <user/drive.h>
#include <user/io.h>
#include <user/types.h>



static const char* drive_type_names[]={
	[DRIVE_TYPE_AHCI]="AHCI",
	[DRIVE_TYPE_ATA]="ATA",
	[DRIVE_TYPE_ATAPI]="ATAPI",
	[DRIVE_TYPE_NVME]="NVMe"
};



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
	const drive_t* drive=drives+i;
	printf("Name: \x1b[1m%s\x1b[0m\nType: \x1b[1m%s\x1b[0m\nSize: \x1b[1m%v\x1b[0m\nBlock size: \x1b[1m%v\x1b[0m\nBlock count: \x1b[1m%lu\x1b[0m\n",
		argv[1],
		drive_type_names[drive->type],
		drive->block_count*drive->block_size,
		drive->block_size,
		drive->block_count
	);
}



DECLARE_COMMAND(drive,"drive <drive>");
