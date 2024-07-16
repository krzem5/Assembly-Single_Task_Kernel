#include <sys/fd/fd.h>
#include <sys/io/io.h>
#include <sys/memory/memory.h>
#include <sys/socket/socket.h>
#include <sys/string/string.h>
#include <sys/types.h>
#include <terminal/terminal.h>



SYS_PUBLIC bool terminal_open_session(const char* path,terminal_session_t* out){
	char buffer[4096];
	if (path){
		sys_string_copy(path,buffer);
	}
	else if (SYS_IS_ERROR(sys_fd_path(sys_io_output_fd,buffer,sizeof(buffer)))&&SYS_IS_ERROR(sys_fd_path(sys_io_error_fd,buffer,sizeof(buffer)))&&SYS_IS_ERROR(sys_fd_path(sys_io_input_fd,buffer,sizeof(buffer)))){
		return 0;
	}
	u32 length=sys_string_length(buffer);
	if (length<4||sys_string_compare(buffer+length-4,"/out")){
		return 0;
	}
	sys_memory_copy("/ctrl",buffer+length-4,6);
	out->ctrl_fd=sys_socket_create(SYS_SOCKET_DOMAIN_UNIX,SYS_SOCKET_TYPE_DGRAM,SYS_SOCKET_PROTOCOL_NONE);
	if (SYS_IS_ERROR(out->ctrl_fd)||SYS_IS_ERROR(sys_socket_connect(out->ctrl_fd,buffer,256))||SYS_IS_ERROR(sys_socket_send(out->ctrl_fd,"abc",3,0))){
		sys_fd_close(out->ctrl_fd);
		out->ctrl_fd=0;
		return 0;
	}
	return 1;
}



SYS_PUBLIC bool terminal_open_session_from_fd(sys_fd_t fd,terminal_session_t* out){
	out->ctrl_fd=fd;
	return 1;
}
