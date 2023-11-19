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
