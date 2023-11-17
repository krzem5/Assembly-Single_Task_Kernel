#include <color.h>
#include <command.h>
#include <cwd.h>
#include <sys/fd.h>
#include <sys/io.h>



static const char* _stat_type_names[]={
	[0]="<unknown>",
	[SYS_FD_STAT_TYPE_FILE]="file",
	[SYS_FD_STAT_TYPE_DIRECTORY]="dir",
	[SYS_FD_STAT_TYPE_LINK]="link",
	[SYS_FD_STAT_TYPE_PIPE]="pipe"
};



void stat_main(int argc,const char*const* argv){
	if (argc<2){
		printf("stat: no input file supplied\n");
		return;
	}
	if (argc>2){
		printf("stat: unrecognized option '%s'\n",argv[2]);
		return;
	}
	s64 fd=sys_fd_open(cwd_fd,argv[1],SYS_FD_FLAG_IGNORE_LINKS|SYS_FD_FLAG_READ);
	if (fd<0){
		printf("stat: unable to open file '%s': error %d\n",argv[1],fd);
		return;
	}
	sys_fd_stat_t stat;
	int error=sys_fd_stat(fd,&stat);
	if (error<0){
		printf("stat: unable to read data from file '%s': error %d\n",argv[1],error);
		goto _cleanup;
	}
	printf("Name: ");
	s64 parent_fd=sys_fd_open(fd,"..",SYS_FD_FLAG_IGNORE_LINKS);
	color_print_file_name(&stat,argv[1],parent_fd,fd);
	sys_fd_close(parent_fd);
	printf("\nType: \x1b[1m%s\x1b[0m\nFlags:\x1b[1m%s\x1b[0m\nPermissions: \x1b[1m%c%c%c%c%c%c%c%c%c\x1b[0m\nSize: \x1b[1m%v (%lu B)\x1b[0m\n",
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
		stat.size
	);
_cleanup:
	sys_fd_close(fd);
}



DECLARE_COMMAND(stat,"stat <file>");
