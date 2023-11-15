#include <sys/fd.h>
#include <sys/io.h>
#include <sys/types.h>



void color_print_file_name(const fd_stat_t* stat,const char* name,s64 parent_fd,s64 fd){
	const char* prefix="";
	if (stat->type==FD_STAT_TYPE_DIRECTORY){
		prefix="\x1b[1;34m";
	}
	else if (stat->type==FD_STAT_TYPE_LINK){
		prefix="\x1b[1;36m";
	}
	else if (stat->type==FD_STAT_TYPE_PIPE){
		prefix="\x1b[33;40m";
	}
	printf("%s%s\x1b[0m",prefix,name);
	if (fd<=0||stat->type!=FD_STAT_TYPE_LINK){
		return;
	}
	printf(" -> ");
	char link_buffer[4096];
	s64 size=fd_read(fd,link_buffer,4095,0);
	if (size<=0){
		printf("???");
		return;
	}
	link_buffer[size]=0;
	int link_fd=fd_open(parent_fd,link_buffer,0);
	fd_stat_t link_stat;
	if (link_fd<0||fd_stat(link_fd,&link_stat)<0){
		printf("\x1b[1;31;40m%s\x1b[0m",link_buffer);
	}
	else{
		color_print_file_name(&link_stat,link_buffer,0,0);
	}
	if (link_fd>=0){
		fd_close(link_fd);
	}
}
