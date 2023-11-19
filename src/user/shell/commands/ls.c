#include <color.h>
#include <command.h>
#include <cwd.h>
#include <string.h>
#include <sys/fd.h>
#include <sys/io.h>
#include <sys/types.h>



static const char* _ls_type_names[]={
	[0]="<unknown>",
	[SYS_FD_STAT_TYPE_FILE]="file",
	[SYS_FD_STAT_TYPE_DIRECTORY]="dir",
	[SYS_FD_STAT_TYPE_LINK]="link",
	[SYS_FD_STAT_TYPE_PIPE]="pipe"
};



static void _list_files(s64 fd){
	for (s64 iter=sys_fd_iter_start(fd);iter>=0;iter=sys_fd_iter_next(iter)){
		char name[256];
		if (sys_fd_iter_get(iter,name,256)<=0){
			continue;
		}
		s64 child=sys_fd_open(fd,name,SYS_FD_FLAG_IGNORE_LINKS|SYS_FD_FLAG_READ);
		if (child<0){
			continue;
		}
		sys_fd_stat_t stat;
		if (sys_fd_stat(child,&stat)<0){
			sys_fd_close(child);
			continue;
		}
		printf("%s\t%v\t",_ls_type_names[stat.type],stat.size);
		color_print_file_name(&stat,name,fd,child);
		putchar('\n');
		sys_fd_close(child);
	}
}



void ls_main(int argc,const char*const* argv){
	const char* directory=NULL;
	for (u32 i=1;i<argc;i++){
		if (argv[i][0]!='-'&&!directory){
			directory=argv[i];
		}
		else{
			printf("ls: unrecognized option '%s'\n",argv[i]);
			return;
		}
	}
	if (!directory){
		_list_files(cwd_fd);
	}
	else{
		s64 fd=sys_fd_open(cwd_fd,directory,0);
		if (fd<0){
			printf("ls: unable to open file '%s': error %d\n",directory,fd);
			return;
		}
		_list_files(fd);
		sys_fd_close(fd);
	}
}



DECLARE_COMMAND(ls,"ls [<directory>|-d|-p]");
