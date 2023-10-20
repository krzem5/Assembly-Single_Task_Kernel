#include <command.h>
#include <cwd.h>
#include <string.h>
#include <user/drive.h>
#include <user/fd.h>
#include <user/io.h>
#include <user/partition.h>
#include <user/types.h>



#define LS_TYPE_FILES 0
#define LS_TYPE_DRIVES 1
#define LS_TYPE_PARTITIONS 2



static void _list_files(s64 fd){
	for (s64 iter=fd_iter_start(fd);iter>=0;iter=fd_iter_next(iter)){
		char name[256];
		if (fd_iter_get(iter,name,256)<=0){
			continue;
		}
		s64 child=fd_open(fd,name,0);
		if (child<0){
			continue;
		}
		fd_stat_t stat;
		if (fd_stat(child,&stat)<0){
			fd_close(child);
			continue;
		}
		printf("\x1b[1m%s\x1b[0m:\t%v\t%s\n",name,stat.size,(stat.type==FD_STAT_TYPE_FILE?"file":"directory"));
		fd_close(child);
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
			printf("\x1b[1m%s\x1b[0m\t%v\t(%s)\n",drive.name,drive.block_count*drive.block_size,drive.model_number);
		}
	}
	else if (type==LS_TYPE_PARTITIONS){
		partition_t partition;
		drive_t drive;
		for (u32 i=0;partition_get(i,&partition)&&drive_get(partition.drive_index,&drive);i++){
			printf("\x1b[1m%s\x1b[0m\t%v\t%s%s%s\n",partition.name,(partition.last_block_index-partition.first_block_index)*drive.block_size,((partition.flags&PARTITION_FLAG_BOOT)?" [boot]":""),((partition.flags&PARTITION_FLAG_HALF_INSTALLED)?" [half-installed]":""),((partition.flags&PARTITION_FLAG_PREVIOUS_BOOT)?" [previous boot]":""));
		}
	}
	else if (!directory){
		_list_files(cwd_fd);
	}
	else{
		s64 fd=fd_open(cwd_fd,directory,0);
		if (fd<0){
			printf("ls: unable to open file '%s': error %d\n",directory,fd);
			return;
		}
		_list_files(fd);
		fd_close(fd);
	}
}



DECLARE_COMMAND(ls,"ls [<directory>|-d|-p]");
