#include <command.h>
#include <cwd.h>
#include <string.h>
#include <user/drive.h>
#include <user/fd.h>
#include <user/handle.h>
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
		handle_iterator_t iterator;
		HANDLE_FOREACH(&iterator,"drive"){
			drive_data_t data;
			if (!drive_get_data(iterator.handle,&data)){
				continue;
			}
			printf("\x1b[1m%s\x1b[0m\t%v\t(%s)\n",data.name,data.block_count*data.block_size,data.model_number);
		}
	}
	else if (type==LS_TYPE_PARTITIONS){
		handle_iterator_t iterator;
		HANDLE_FOREACH(&iterator,"partition"){
			partition_data_t data;
			if (!partition_get_data(iterator.handle,&data)){
				continue;
			}
			drive_data_t drive_data;
			if (!drive_get_data(data.drive_handle,&drive_data)){
				continue;
			}
			printf("\x1b[1m%s\x1b[0m\t(%s)\t%v\n",data.name,data.partition_table_name,(data.end_lba-data.start_lba)*drive_data.block_size);
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
