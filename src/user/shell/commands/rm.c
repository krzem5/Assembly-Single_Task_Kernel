#include <command.h>
#include <cwd.h>
#include <string.h>
#include <user/fs.h>
#include <user/io.h>



static void _delete_recursive(int fd){
	int child=fs_get_relative(fd,FS_RELATIVE_FIRST_CHILD,0);
	fs_stat_t stat;
	while (child>=0){
		if (fs_stat(child,&stat)<0){
			fs_close(child);
			break;
		}
		if (stat.type==FS_STAT_TYPE_DIRECTORY){
			_delete_recursive(child);
		}
		int next_child=fs_get_relative(child,FS_RELATIVE_NEXT_SIBLING,0);
		fs_delete(child);
		child=next_child;
	}
}



void rm_main(int argc,const char*const* argv){
	_Bool recursive=0;
	const char* file=NULL;
	for (u32 i=1;i<argc;i++){
		if (i<argc-1&&string_equal(argv[i],"-r")){
			recursive=1;
		}
		else if (argv[i][0]!='-'&&!file){
			file=argv[i];
		}
		else{
			printf("rm: unrecognized option '%s'\n",argv[i]);
			return;
		}
	}
	int fd=fs_open(cwd_fd,file,0);
	if (fd<0){
		printf("rm: unable to open file '%s': error %d\n",file,fd);
		return;
	}
	if (!recursive){
		int error=fs_delete(fd);
		if (error<0){
			printf("rm: unable to delete file '%s': error %d\n",file,error);
			fs_close(fd);
			return;
		}
		return;
	}
	_delete_recursive(fd);
	fs_delete(fd);
}



DECLARE_COMMAND(rm,"rm <drive>");
