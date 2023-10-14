#include <command.h>
#include <cwd.h>
#include <user/fs.h>
#include <user/io.h>
#include <user/partition.h>



static const char* fs_stat_type_names[]={
	[FS_STAT_TYPE_FILE]="file",
	[FS_STAT_TYPE_DIRECTORY]="directory"
};



void stat_main(int argc,const char*const* argv){
	if (argc<2){
		printf("stat: no input file supplied\n");
		return;
	}
	if (argc>2){
		printf("stat: unrecognized option '%s'\n",argv[2]);
		return;
	}
	int fd=fs_open(cwd_fd,argv[1],0);
	if (fd<0){
		printf("stat: unable to open file '%s': error %d\n",argv[1],fd);
		return;
	}
	fs_stat_t stat;
	int error=fs_stat(fd,&stat);
	if (error<0){
		printf("stat: unable to read data from file '%s': error %d\n",argv[1],error);
		goto _cleanup;
	}
	partition_t partition;
	if (!partition_get(stat.fs_index,&partition)){
		printf("stat: unable to read partition data\n");
		goto _cleanup;
	}
	printf("Name: \x1b[1m%s\x1b[0m\nType: \x1b[1m%s\x1b[0m\nSize: \x1b[1m%v (%lu B)\x1b[0m\nPartition: \x1b[1m%s\x1b[0m\nID: \x1b[1m%p\x1b[0m\n",stat.name,fs_stat_type_names[stat.type],stat.size,stat.size,partition.name,stat.node_id);
_cleanup:
	fs_close(fd);
}



DECLARE_COMMAND(stat,"stat <file>");
