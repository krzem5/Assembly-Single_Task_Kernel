#include <dircolor/dircolor.h>
#include <sys2/error/error.h>
#include <sys2/fd/fd.h>
#include <sys2/io/io.h>
#include <sys2/types.h>
#include <sys2/util/options.h>



static const char* _ls_type_names[]={
	[0]="<unknown>",
	[SYS2_FD_STAT_TYPE_FILE]="file",
	[SYS2_FD_STAT_TYPE_DIRECTORY]="dir",
	[SYS2_FD_STAT_TYPE_LINK]="link",
	[SYS2_FD_STAT_TYPE_PIPE]="pipe",
	[SYS2_FD_STAT_TYPE_SOCKET]="socket"
};



int main(int argc,const char** argv){
	u32 i=sys2_options_parse(argc,argv,NULL);
	if (!i){
		return 1;
	}
	const char* directory=(i<argc?argv[i]:".");
	sys2_fd_t fd=sys2_fd_open(0,directory,0);
	if (SYS2_IS_ERROR(fd)){
		sys2_io_print("ls: unable to open file '%s': error %d\n",directory,fd);
		return 1;
	}
	for (sys2_fd_iterator_t iter=sys2_fd_iter_start(fd);!SYS2_IS_ERROR(iter);iter=sys2_fd_iter_next(iter)){
		char name[256];
		if (SYS2_IS_ERROR(sys2_fd_iter_get(iter,name,256))){
			continue;
		}
		sys2_fd_t child=sys2_fd_open(fd,name,SYS2_FD_FLAG_IGNORE_LINKS|SYS2_FD_FLAG_READ);
		if (SYS2_IS_ERROR(child)){
			continue;
		}
		sys2_fd_stat_t stat;
		if (sys2_fd_stat(child,&stat)<0){
			sys2_fd_close(child);
			continue;
		}
		sys2_io_print("%s\t%v\t",_ls_type_names[stat.type],stat.size);
		dircolor_get_color_with_link(&stat,name,child);
		sys2_io_print("\n");
		sys2_fd_close(child);
	}
	sys2_fd_close(fd);
	return 0;
}
