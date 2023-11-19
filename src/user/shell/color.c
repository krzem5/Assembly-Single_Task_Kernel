#include <dircolor/dircolor.h>
#include <sys/fd.h>
#include <sys/io.h>
#include <sys/types.h>



void color_print_file_name(const sys_fd_stat_t* stat,const char* name,s64 parent_fd,s64 fd){
	char prefix[32];
	dircolor_get_color(stat,prefix);
	printf("%s%s\x1b[0m",prefix,name);
	if (fd<=0||stat->type!=SYS_FD_STAT_TYPE_LINK){
		return;
	}
	printf(" -> ");
	char link_buffer[4096];
	s64 size=sys_fd_read(fd,link_buffer,4095,0);
	if (size<=0){
		printf("???");
		return;
	}
	link_buffer[size]=0;
	s64 link_fd=sys_fd_open(parent_fd,link_buffer,0);
	sys_fd_stat_t link_stat;
	if (link_fd<0||sys_fd_stat(link_fd,&link_stat)<0){
		printf("\x1b[1;31;40m%s\x1b[0m",link_buffer);
	}
	else{
		color_print_file_name(&link_stat,link_buffer,0,0);
	}
	if (link_fd>=0){
		sys_fd_close(link_fd);
	}
}
