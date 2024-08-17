#include <sys/error/error.h>
#include <sys/socket/socket.h>
#include <sys/types.h>
#include <terminal/protocol.h>
#include <terminal/server.h>
#include <terminal/session.h>
#include <sys/io/io.h>



SYS_PUBLIC bool terminal_server_process_packet(terminal_session_t* session,terminal_server_state_t* state){
	u8 buffer[256];
	u32 length=terminal_session_recv_packet(session,buffer,sizeof(buffer),TERMINAL_PROTOCOL_MESSAGE_TYPE_CLIENT_ANY);
	if (!length){
		return 0;
	}
	if (buffer[0]==TERMINAL_PROTOCOL_MESSAGE_TYPE_CLIENT_GET_VERSION&&length==1){
		buffer[0]=TERMINAL_PROTOCOL_MESSAGE_TYPE_SERVER_VERSION;
		buffer[1]=0;
		buffer[2]=1;
		return terminal_session_send_packet(session,buffer,3);
	}
	if (buffer[0]==TERMINAL_PROTOCOL_MESSAGE_TYPE_CLIENT_SET_FLAGS&&length==9){
		if (!state->flag_update_callback){
			goto _skip_flag_update;
		}
		u32 new_flags=(state->flags&(~(*((const u32*)(buffer+1)))))|(*((const u32*)(buffer+5)));
		u32 clear=state->flags&(~new_flags);
		u32 set=(~state->flags)&new_flags;
		if (clear||set){
			state->flags=new_flags&(~state->flag_update_callback(clear,set,new_flags));
		}
_skip_flag_update:
		buffer[0]=TERMINAL_PROTOCOL_MESSAGE_TYPE_SERVER_FLAGS;
		*((u32*)(buffer+1))=state->flags;
		return terminal_session_send_packet(session,buffer,5);
	}
	if (buffer[0]==TERMINAL_PROTOCOL_MESSAGE_TYPE_CLIENT_GET_SIZE&&length==1){
		u32 size[2]={0,0};
		if (state->size_inquiry_callback){
			state->size_inquiry_callback(size);
		}
		buffer[0]=TERMINAL_PROTOCOL_MESSAGE_TYPE_SERVER_SIZE;
		*((u32*)(buffer+1))=size[0];
		*((u32*)(buffer+5))=size[1];
		return terminal_session_send_packet(session,buffer,9);
	}
	return 1;
}
