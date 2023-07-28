#include <command.h>
#include <cwd.h>
#include <user/fs.h>
#include <user/io.h>
#include <user/types.h>



#define COPY_BUFFER_SIZE 4096



static u8 _copy_buffer[COPY_BUFFER_SIZE];



static void _copy_file(int src_fd,int dst_fd,u64 size){
	for (u64 i=0;i<size;i+=COPY_BUFFER_SIZE){
		u64 j=size-i;
		if (j>COPY_BUFFER_SIZE){
			j=COPY_BUFFER_SIZE;
		}
		if (fs_read(src_fd,_copy_buffer,j)!=j||fs_write(dst_fd,_copy_buffer,j)!=j){
			return;
		}
	}
}



static void _recursive_copy(int src_fd,int dst_dir_fd){
	fs_stat_t stat;
	if (fs_stat(dst_dir_fd,&stat)<0){
		return;
	}
	int dst_fd=fs_open(dst_dir_fd,stat.name,FS_FLAG_CREATE|(stat.type==FS_STAT_TYPE_DIRECTORY?FS_FLAG_DIRECTORY:0));
	if (stat.type==FS_STAT_TYPE_FILE){
		_copy_file(src_fd,dst_fd,stat.size);
	}
	else{
		printf("Unimplemented\n");
	}
	fs_close(dst_fd);
}



void cp_main(int argc,const char*const* argv){
	if (argc<2){
		printf("cp: no input file supplied\n");
		return;
	}
	if (argc<3){
		printf("cp: no output file supplied\n");
		return;
	}
	if (argc>3){
		printf("cp: unrecognized option '%s'\n",argv[3]);
		return;
	}
	int src_fd=fs_open(cwd_fd,argv[1],0);
	if (src_fd<0){
		printf("cp: unable to open file '%s': error %d\n",argv[1],src_fd);
		return;
	}
	fs_stat_t src_stat;
	int error=fs_stat(src_fd,&src_stat);
	if (error<0){
		fs_close(src_fd);
		printf("cp: unable to read data from file '%s': error %d\n",argv[1],error);
		return;
	}
	int dst_fd=fs_open(cwd_fd,argv[2],FS_FLAG_CREATE|(src_stat.type==FS_STAT_TYPE_DIRECTORY?FS_FLAG_DIRECTORY:0));
	if (dst_fd<0){
		fs_close(src_fd);
		printf("cp: unable to open file '%s': error %d\n",argv[2],dst_fd);
		return;
	}
	fs_stat_t dst_stat;
	error=fs_stat(dst_fd,&dst_stat);
	if (error<0){
		printf("cp: unable to read data from file '%s': error %d\n",argv[2],error);
		goto _cleanup;
	}
	if (src_stat.type!=dst_stat.type){
		printf("cp: source and destination have different types\n");
		goto _cleanup;
	}
	if (src_stat.type==FS_STAT_TYPE_FILE){
		_copy_file(src_fd,dst_fd,src_stat.size);
	}
	else{
		_recursive_copy(src_fd,dst_fd);
	}
_cleanup:
	fs_close(src_fd);
	fs_close(dst_fd);
}



DECLARE_COMMAND(cp,"cp <source_file> (<destination_directory> | <destination_file>)");
