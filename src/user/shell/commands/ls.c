#include <command.h>
#include <cwd.h>
#include <string.h>
#include <user/drive.h>
#include <user/fs.h>
#include <user/io.h>
#include <user/partition.h>
#include <user/types.h>



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
	[PARTITION_TYPE_KFS]="KFS"
};



static void _list_files(int fd){
	int child=fs_get_relative(fd,FS_RELATIVE_FIRST_CHILD,0);
	fs_stat_t stat;
	while (child>=0){
		if (fs_stat(child,&stat)<0){
			fs_close(child);
			break;
		}
		printf("\x1b[1m%s\x1b[0m:\t%v\t%s\n",stat.name,stat.size,(stat.type==FS_STAT_TYPE_FILE?"file":"directory"));
		int next_child=fs_get_relative(child,FS_RELATIVE_NEXT_SIBLING,0);
		fs_close(child);
		child=next_child;
	}
}



void ls_main(int argc,const char*const* argv){
	u8 type=LS_TYPE_FILES;
	const char* directory=NULL;
	for (u32 i=1;i<argc;i++){
		if (string_equal(argv[i],"-d")){
			type=LS_TYPE_DRIVES;
		}
		else if (string_equal(argv[i],"-p")){
			type=LS_TYPE_PARTITIONS;
		}
		else if (argv[i][0]!='-'&&!directory){
			directory=argv[i];
		}
		else{
			printf("ls: unrecognized option '%s'\n",argv[i]);
			return;
		}
	}
	if (type!=LS_TYPE_FILES&&directory){
		printf("ls: directory supplied during drive or partition enumeration\n");
		return;
	}
	if (type==LS_TYPE_DRIVES){
		drive_t drive;
		for (u32 i=0;drive_get(i,&drive);i++){
			printf("\x1b[1m%s\x1b[0m\t%v\t%s\t(%s)%s\n",drive.name,drive.block_count*drive.block_size,drive_type_names[drive.type],drive.model_number,((drive.flags&DRIVE_FLAG_BOOT)?" [boot]":""));
		}
	}
	else if (type==LS_TYPE_PARTITIONS){
		partition_t partition;
		drive_t drive;
		for (u32 i=0;partition_get(i,&partition)&&drive_get(partition.drive_index,&drive);i++){
			printf("\x1b[1m%s\x1b[0m\t%v\t%s\t%s%s%s\n",partition.name,(partition.last_block_index-partition.first_block_index)*drive.block_size,partition_type_names[partition.type],((partition.flags&PARTITION_FLAG_BOOT)?" [boot]":""),((partition.flags&PARTITION_FLAG_HALF_INSTALLED)?" [half-installed]":""),((partition.flags&PARTITION_FLAG_PREVIOUS_BOOT)?" [previous boot]":""));
		}
	}
	else if (!directory){
		_list_files(cwd_fd);
	}
	else{
		int fd=fs_open(cwd_fd,directory,0);
		if (fd<0){
			printf("ls: unable to open file '%s': error %d\n",directory,fd);
			return;
		}
		_list_files(fd);
		fs_close(fd);
	}
}



DECLARE_COMMAND(ls,"ls [<directory>|-d|-p]");
