#include <dircolor/dircolor.h>
#include <sys/error/error.h>
#include <sys/fd/fd.h>
#include <sys/io/io.h>
#include <sys/types.h>
#include <sys/util/options.h>



static const char* _ls_type_names[]={
	[0]="<unknown>",
	[SYS_FD_STAT_TYPE_FILE]="file",
	[SYS_FD_STAT_TYPE_DIRECTORY]="dir",
	[SYS_FD_STAT_TYPE_LINK]="link",
	[SYS_FD_STAT_TYPE_PIPE]="pipe",
	[SYS_FD_STAT_TYPE_SOCKET]="socket"
};



int main(int argc,const char** argv){
	const char* path=".";
	if (!sys_options_parse(argc,argv,"{:p:path}s",&path)){
		return 1;
	}
	sys_fd_t fd=sys_fd_open(0,path,0);
	if (SYS_IS_ERROR(fd)){
		sys_io_print("ls: unable to open file '%s': error %d\n",path,fd);
		return 1;
	}
	for (sys_fd_iterator_t iter=sys_fd_iter_start(fd);!SYS_IS_ERROR(iter);iter=sys_fd_iter_next(iter)){
		char name[256];
		if (SYS_IS_ERROR(sys_fd_iter_get(iter,name,256))){
			continue;
		}
		sys_fd_t child=sys_fd_open(fd,name,SYS_FD_FLAG_IGNORE_LINKS|SYS_FD_FLAG_READ);
		if (SYS_IS_ERROR(child)){
			continue;
		}
		sys_fd_stat_t stat;
		if (sys_fd_stat(child,&stat)<0){
			sys_fd_close(child);
			continue;
		}
		sys_io_print("%s\t%v\t",_ls_type_names[stat.type],stat.size);
		dircolor_get_color_with_link(&stat,name,child);
		sys_io_print("\n");
		sys_fd_close(child);
	}
	sys_fd_close(fd);
	return 0;
}
