#include <sys/error/error.h>
#include <sys/socket/socket.h>
#include <sys/types.h>
#include <terminal/client.h>
#include <terminal/protocol.h>
#include <terminal/session.h>



SYS_PUBLIC u16 terminal_client_get_version(terminal_session_t* session){
	u8 buffer[3]={
		TERMINAL_PROTOCOL_MESSAGE_TYPE_CLIENT_GET_VERSION,
	};
	return (terminal_session_send_packet(session,buffer,1)&&terminal_session_recv_packet(session,buffer,3,TERMINAL_PROTOCOL_MESSAGE_TYPE_SERVER_VERSION)==3?(buffer[1]<<8)|buffer[2]:0);
}



SYS_PUBLIC u32 terminal_client_get_flags(terminal_session_t* session){
	return terminal_client_set_flags(session,0,0);
}



SYS_PUBLIC u32 terminal_client_set_flags(terminal_session_t* session,u32 clear,u32 set){
	u8 buffer[9]={
		TERMINAL_PROTOCOL_MESSAGE_TYPE_CLIENT_SET_FLAGS,
		clear,clear>>8,clear>>16,clear>>24,
		set,set>>8,set>>16,set>>24,
	};
	return (terminal_session_send_packet(session,buffer,9)&&terminal_session_recv_packet(session,buffer,5,TERMINAL_PROTOCOL_MESSAGE_TYPE_SERVER_FLAGS)==5?*((const u32*)(buffer+1)):0);
}
