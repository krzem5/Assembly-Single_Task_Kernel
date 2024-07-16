#include <sys/error/error.h>
#include <sys/fd/fd.h>
#include <sys/io/io.h>
#include <sys/memory/memory.h>
#include <sys/socket/socket.h>
#include <sys/string/string.h>
#include <sys/types.h>
#include <terminal/session.h>



SYS_PUBLIC bool terminal_session_open(const char* path,terminal_session_t* out){
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
	out->ctrl_fd=sys_fd_open(0,buffer,SYS_FD_FLAG_READ|SYS_FD_FLAG_WRITE);
	if (SYS_IS_ERROR(out->ctrl_fd)){
		out->ctrl_fd=0;
		return 0;
	}
	return 1;
}



SYS_PUBLIC bool terminal_session_open_from_fd(sys_fd_t fd,terminal_session_t* out){
	out->ctrl_fd=fd;
	return 1;
}



SYS_PUBLIC void terminal_session_close(terminal_session_t* session){
	sys_fd_close(session->ctrl_fd);
	session->ctrl_fd=0;
}



SYS_PUBLIC u32 terminal_session_recv_packet(terminal_session_t* session,void* buffer,u32 buffer_size,u8 type){
	if (!buffer_size){
		return 0;
	}
	while (1){
		sys_error_t length=sys_socket_recv(session->ctrl_fd,buffer,buffer_size,0);
		if (!length){
			continue;
		}
		if (SYS_IS_ERROR(length)){
			if (length==SYS_ERROR_NO_SPACE){
				continue;
			}
			return 0;
		}
		u8 msg_type=*((const u8*)buffer);
		if (((type^msg_type)&0x80)||((type&0x7f)&&msg_type!=type)){
			continue;
		}
		return length;
	}
}



SYS_PUBLIC bool terminal_session_send_packet(terminal_session_t* session,const void* buffer,u32 buffer_size){
	return !SYS_IS_ERROR(sys_socket_send(session->ctrl_fd,buffer,buffer_size,0));
}
