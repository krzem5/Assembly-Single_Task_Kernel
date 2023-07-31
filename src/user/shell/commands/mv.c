#include <command.h>
#include <cwd.h>
#include <user/fs.h>
#include <user/io.h>



void mv_main(int argc,const char*const* argv){
	if (argc<2){
		printf("mv: no input file supplied\n");
		return;
	}
	if (argc<3){
		printf("mv: no output file supplied\n");
		return;
	}
	if (argc>3){
		printf("mv: unrecognized option '%s'\n",argv[3]);
		return;
	}
	int src_fd=fs_open(cwd_fd,argv[1],FS_FLAG_READ);
	if (src_fd<0){
		printf("mv: unable to open file '%s': error %d\n",argv[1],src_fd);
		return;
	}
	int dst_fd=fs_open(cwd_fd,argv[2],0);
	if (dst_fd>=0){
		fs_close(dst_fd);
		printf("mv: file '%s' already exists\n",argv[2]);
		goto _cleanup;
	}
	fs_stat_t stat;
	int error=fs_stat(src_fd,&stat);
	if (error<0){
		printf("mv: unable to read data from file '%s': error %d\n",argv[1],error);
		goto _cleanup;
	}
	dst_fd=fs_open(cwd_fd,argv[2],FS_FLAG_CREATE|(stat.type==FS_STAT_TYPE_DIRECTORY?FS_FLAG_DIRECTORY:0));
	error=fs_move(src_fd,dst_fd);
	if (error<0){
		printf("mv: unable to move file: error %d\n",error);
	}
	fs_close(dst_fd);
	error=fs_delete(src_fd);
	if (error<0){
		printf("mv: unable to delete file '%s': error %d\n",argv[1],error);
		fs_close(src_fd);
		return;
	}
_cleanup:
	fs_close(src_fd);
}



DECLARE_COMMAND(mv,"mv <source> <destination>");
