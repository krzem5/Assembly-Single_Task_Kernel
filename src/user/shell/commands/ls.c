#include <command.h>
#include <string.h>
#include <user/drive.h>
#include <user/fs.h>
#include <user/io.h>
#include <user/partition.h>



#define LS_TYPE_FILES 0
#define LS_TYPE_DRIVES 1
#define LS_TYPE_PARTITIONS 2



static const char* drive_type_names[]={
	[DRIVE_TYPE_AHCI]="AHCI",
	[DRIVE_TYPE_ATA]="ATA",
	[DRIVE_TYPE_ATAPI]="ATAPI",
	[DRIVE_TYPE_NVME]="NVMe"
};



static const char* partition_type_names[]={
	[PARTITION_TYPE_DRIVE]="drive",
	[PARTITION_TYPE_EMPTY]="empty",
	[PARTITION_TYPE_ISO9660]="ISO 9660",
	[PARTITION_TYPE_GPT]="GPT",
};



void ls_main(int argc,const char*const* argv){
	u8 type=LS_TYPE_FILES;
	for (u32 i=1;i<argc;i++){
		if (string_equal(argv[i],"-d")){
			type=LS_TYPE_DRIVES;
		}
		else if (string_equal(argv[i],"-p")){
			type=LS_TYPE_PARTITIONS;
		}
		else{
			printf("ls: unrecognized option '%s'\n",argv[i]);
			return;
		}
	}
	if (type==LS_TYPE_DRIVES){
		const drive_t* drive=drives;
		for (u32 i=0;i<drive_count;i++){
			printf("\x1b[1m%s\x1b[0m:\t%v\t%s\t(%s)%s\n",drive->name,drive->block_count*drive->block_size,drive_type_names[drive->type],drive->model_number,((drive->flags&DRIVE_FLAG_BOOT)?" [boot]":""));
			drive++;
		}
	}
	else if (type==LS_TYPE_PARTITIONS){
		const partition_t* partition=partitions;
		for (u32 i=0;i<partition_count;i++){
			printf("\x1b[1m%s\x1b[0m:\t%v\t%s%s\n",partition->name,(partition->last_block_index-partition->first_block_index)*(drives+partition->drive_index)->block_size,partition_type_names[partition->type],((partition->flags&DRIVE_FLAG_BOOT)?" [boot]":""));
			partition++;
		}
	}
	else{
		//
	}
}



DECLARE_COMMAND(ls,"ls [-d|-p]");
