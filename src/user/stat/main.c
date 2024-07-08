#include <dircolor/dircolor.h>
#include <sys/error/error.h>
#include <sys/fd/fd.h>
#include <sys/id/group.h>
#include <sys/id/user.h>
#include <sys/io/io.h>
#include <sys/string/string.h>
#include <sys/types.h>
#include <sys/util/options.h>



static const char* _stat_type_names[]={
	[0]="<unknown>",
	[SYS_FD_STAT_TYPE_FILE]="file",
	[SYS_FD_STAT_TYPE_DIRECTORY]="dir",
	[SYS_FD_STAT_TYPE_LINK]="link",
	[SYS_FD_STAT_TYPE_PIPE]="pipe",
	[SYS_FD_STAT_TYPE_SOCKET]="socket"
};



static bool _check_if_link(const char* path){
	sys_fd_t fd=sys_fd_open(0,path,SYS_FD_FLAG_FIND_LINKS);
	if (fd==SYS_ERROR_LINK_FOUND){
		return 1;
	}
	sys_fd_close(fd);
	return 0;
}



static const char* _get_lock_type(sys_handle_t handle){
	if (!handle){
		return "none";
	}
	char buffer[256];
	if (SYS_IS_ERROR(sys_handle_get_name(handle,buffer,sizeof(buffer)))){
		return "<unknown>";
	}
	if (!sys_string_compare(buffer,"kernel.thread")){
		return "thread";
	}
	if (!sys_string_compare(buffer,"kernel.process")){
		return "process";
	}
	if (!sys_string_compare(buffer,"kernel.process.group")){
		return "process_group";
	}
	return "???";
}



int main(int argc,const char** argv){
	u64 i=sys_options_parse(argc,argv,NULL);
	if (!i){
		return 1;
	}
	for (;i<argc;i++){
		sys_fd_t fd=sys_fd_open(0,argv[i],0);
		if (SYS_IS_ERROR(fd)){
			sys_io_print("stat: unable to open file '%s': error %d\n",argv[i],fd);
			return 1;
		}
		sys_fd_stat_t stat;
		sys_error_t error=sys_fd_stat(fd,&stat);
		if (SYS_IS_ERROR(error)){
			sys_io_print("stat: unable to read data from file '%s': error %d\n",argv[i],error);
			sys_fd_close(fd);
			return 1;
		}
		sys_io_print("Name: ");
		if (_check_if_link(argv[i])){
			sys_fd_stat_t tmp_stat={
				.type=SYS_FD_STAT_TYPE_LINK
			};
			char buffer[32];
			dircolor_get_color(&tmp_stat,buffer);
			sys_io_print("%s%s\x1b[0m -> ",buffer,argv[i]);
		}
		char buffer[32];
		dircolor_get_color(&stat,buffer);
		char path_buffer[4096];
		if (SYS_IS_ERROR(sys_fd_path(fd,path_buffer,sizeof(path_buffer)))){
			sys_string_copy(argv[i],path_buffer);
		}
		sys_io_print("%s%s\x1b[0m",buffer,path_buffer);
		char uid_name_buffer[256]="???";
		sys_uid_get_name(stat.uid,uid_name_buffer,256);
		char gid_name_buffer[256]="???";
		sys_gid_get_name(stat.gid,gid_name_buffer,256);
		sys_io_print("\nType: \x1b[1m%s\x1b[0m\nFlags:\x1b[1m%s\x1b[0m\nPermissions: \x1b[1m%c%c%c%c%c%c%c%c%c\x1b[0m\nSize: \x1b[1m%v\x1b[0m (\x1b[1m%lu B\x1b[0m)\nUid: \x1b[1m%s\x1b[0m (\x1b[1m%u\x1b[0m)\x1b[0m\nGid: \x1b[1m%s\x1b[0m (\x1b[1m%u\x1b[0m)\nLock: \x1b[1m%s\x1b[0m (\x1b[1m%p\x1b[0m)\nAccess: \x1b[1m%t\x1b[0m\nModify: \x1b[1m%t\x1b[0m\nChange: \x1b[1m%t\x1b[0m\nBirth: \x1b[1m",
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
			_get_lock_type(stat.lock_handle),
			stat.lock_handle,
			stat.time_access,
			stat.time_modify,
			stat.time_change
		);
		if (stat.time_birth){
			sys_io_print("%t\x1b[0m\n",stat.time_birth);
		}
		else{
			sys_io_print("?\x1b[0m\n");
		}
		sys_fd_close(fd);
	}
	return 0;
}
