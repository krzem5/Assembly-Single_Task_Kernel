#include <command.h>
#include <cwd.h>
#include <user/fd.h>
#include <user/io.h>
#include <user/partition.h>



static const char* _fd_stat_type_names[]={
	[FD_STAT_TYPE_FILE]="file",
	[FD_STAT_TYPE_DIRECTORY]="directory"
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
	s64 fd=fd_open(cwd_fd,argv[1],0);
	if (fd<0){
		printf("stat: unable to open file '%s': error %d\n",argv[1],fd);
		return;
	}
	fd_stat_t stat;
	int error=fd_stat(fd,&stat);
	if (error<0){
		printf("stat: unable to read data from file '%s': error %d\n",argv[1],error);
		goto _cleanup;
	}
	partition_t partition;
	if (!partition_get(/*stat.fs_index*/1,&partition)){
		printf("stat: unable to read partition data\n");
		goto _cleanup;
	}
	printf("Name: \x1b[1m%s\x1b[0m\nType: \x1b[1m%s\x1b[0m\nSize: \x1b[1m%v (%lu B)\x1b[0m\nPartition: \x1b[1m%s\x1b[0m\nID: \x1b[1m%p\x1b[0m\n",stat.name,_fd_stat_type_names[stat.type],stat.size,stat.size,partition.name,0x1122334455667788ull);
_cleanup:
	fd_close(fd);
}



DECLARE_COMMAND(stat,"stat <file>");
