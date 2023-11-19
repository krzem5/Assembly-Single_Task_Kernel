#include <sys/fd.h>
#include <sys/io.h>
#include <sys/types.h>



SYS_PUBLIC void dircolor_init(void){
	return;
}



SYS_PUBLIC void dircolor_get_color(const sys_fd_stat_t* stat,char* buffer){
	if (stat->type==SYS_FD_STAT_TYPE_DIRECTORY){
		buffer[0]=0x1b;
		buffer[1]='[';
		buffer[2]='1';
		buffer[3]=';';
		buffer[4]='3';
		buffer[5]='4';
		buffer[6]='m';
		buffer[7]=0;
	}
	else if (stat->type==SYS_FD_STAT_TYPE_LINK){
		buffer[0]=0x1b;
		buffer[1]='[';
		buffer[2]='1';
		buffer[3]=';';
		buffer[4]='3';
		buffer[5]='6';
		buffer[6]='m';
		buffer[7]=0;
	}
	else if (stat->type==SYS_FD_STAT_TYPE_PIPE){
		buffer[0]=0x1b;
		buffer[1]='[';
		buffer[2]='3';
		buffer[3]='3';
		buffer[4]=';';
		buffer[5]='4';
		buffer[6]='0';
		buffer[7]='m';
		buffer[8]=0;
	}
	else if (stat->permissions&0111){
		buffer[0]=0x1b;
		buffer[1]='[';
		buffer[2]='1';
		buffer[3]=';';
		buffer[4]='3';
		buffer[5]='2';
		buffer[6]='m';
		buffer[7]=0;
	}
	else{
		buffer[0]=0;
	}
}



SYS_PUBLIC void dircolor_get_color_with_link(const sys_fd_stat_t* stat,const char* name,s64 fd){
	char prefix[32];
	dircolor_get_color(stat,prefix);
	printf("%s%s\x1b[0m",prefix,name);
	if (stat->type!=SYS_FD_STAT_TYPE_LINK){
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
	s64 parent_fd=sys_fd_open(fd,"..",SYS_FD_FLAG_IGNORE_LINKS);
	s64 link_fd=sys_fd_open(parent_fd,link_buffer,0);
	sys_fd_close(parent_fd);
	sys_fd_stat_t link_stat;
	if (link_fd<0||sys_fd_stat(link_fd,&link_stat)<0){
		printf("\x1b[1;31;40m%s\x1b[0m",link_buffer);
	}
	else{
		dircolor_get_color(&link_stat,prefix);
		printf("%s%s\x1b[0m",prefix,link_buffer);
	}
	if (link_fd>=0){
		sys_fd_close(link_fd);
	}
}
