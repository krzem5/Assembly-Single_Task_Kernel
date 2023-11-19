#include <dircolor/dircolor.h>
#include <sys/fd.h>
#include <sys/io.h>
#include <sys/options.h>
#include <sys/syscall.h>



static const char* _stat_type_names[]={
	[0]="<unknown>",
	[SYS_FD_STAT_TYPE_FILE]="file",
	[SYS_FD_STAT_TYPE_DIRECTORY]="dir",
	[SYS_FD_STAT_TYPE_LINK]="link",
	[SYS_FD_STAT_TYPE_PIPE]="pipe"
};



int main(int argc,const char** argv){
	u64 i=sys_options_parse(argc,argv,NULL);
	if (!i){
		return 1;
	}
	for (;i<argc;i++){
		s64 fd=sys_fd_open(0,argv[i],SYS_FD_FLAG_IGNORE_LINKS|SYS_FD_FLAG_READ);
		if (fd<0){
			printf("stat: unable to open file '%s': error %d\n",argv[i],fd);
			return 1;
		}
		sys_fd_stat_t stat;
		int error=sys_fd_stat(fd,&stat);
		if (error<0){
			printf("stat: unable to read data from file '%s': error %d\n",argv[i],error);
			sys_fd_close(fd);
			return 1;
		}
		printf("Name: ");
		dircolor_get_color_with_link(&stat,argv[i],fd);
		char uid_name_buffer[256]="???";
		_syscall_uid_get_name(stat.uid,uid_name_buffer,256);
		char gid_name_buffer[256]="???";
		_syscall_gid_get_name(stat.gid,gid_name_buffer,256);
		printf("\nType: \x1b[1m%s\x1b[0m\nFlags:\x1b[1m%s\x1b[0m\nPermissions: \x1b[1m%c%c%c%c%c%c%c%c%c\x1b[0m\nSize: \x1b[1m%v\x1b[0m (\x1b[1m%lu B\x1b[0m)\nUid: \x1b[1m%s\x1b[0m (\x1b[1m%u\x1b[0m)\x1b[0m\nGid: \x1b[1m%s\x1b[0m (\x1b[1m%u\x1b[0m)\nAccess: \x1b[1m%t\x1b[0m\nModify: \x1b[1m%t\x1b[0m\nChange: \x1b[1m%t\x1b[0m\nBirth: \x1b[1m%t\x1b[0m\n",
			_stat_type_names[stat.type],
			((stat.flags&SYS_FD_STAT_FLAG_VIRTUAL)?" virtual":""),
			((stat.permissions&SYS_FD_PERMISSION_ROOT_READ)?'r':'-'),
			((stat.permissions&SYS_FD_PERMISSION_ROOT_WRITE)?'w':'-'),
			((stat.permissions&SYS_FD_PERMISSION_ROOT_EXEC)?'x':'-'),
			((stat.permissions&SYS_FD_PERMISSION_USER_READ)?'r':'-'),
			((stat.permissions&SYS_FD_PERMISSION_USER_WRITE)?'w':'-'),
			((stat.permissions&SYS_FD_PERMISSION_USER_EXEC)?'x':'-'),
			((stat.permissions&SYS_FD_PERMISSION_OTHER_READ)?'r':'-'),
			((stat.permissions&SYS_FD_PERMISSION_OTHER_WRITE)?'w':'-'),
			((stat.permissions&SYS_FD_PERMISSION_OTHER_EXEC)?'x':'-'),
			stat.size,
			stat.size,
			uid_name_buffer,
			stat.uid,
			gid_name_buffer,
			stat.gid,
			stat.time_access,
			stat.time_modify,
			stat.time_change,
			stat.time_birth
		);
		sys_fd_close(fd);
	}
	return 0;
}
