#include <dircolor/dircolor.h>
#include <sys/fd.h>
#include <sys/io.h>
#include <sys/options.h>
#include <sys/types.h>



static const char* _ls_type_names[]={
	[0]="<unknown>",
	[SYS_FD_STAT_TYPE_FILE]="file",
	[SYS_FD_STAT_TYPE_DIRECTORY]="dir",
	[SYS_FD_STAT_TYPE_LINK]="link",
	[SYS_FD_STAT_TYPE_PIPE]="pipe",
	[SYS_FD_STAT_TYPE_SOCKET]="socket"
};



int main(int argc,const char** argv){
	dircolor_init();
	u32 i=sys_options_parse(argc,argv,NULL);
	if (!i){
		return 1;
	}
	const char* directory=(i<argc?argv[i]:".");
	s64 fd=sys_fd_open(0,directory,0);
	if (fd<0){
		printf("ls: unable to open file '%s': error %d\n",directory,fd);
		return 1;
	}
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
		dircolor_get_color_with_link(&stat,name,child);
		putchar('\n');
		sys_fd_close(child);
	}
	sys_fd_close(fd);
	return 0;
}
