#include <user/io.h>
#include <user/layer3.h>
#include <user/network.h>
#include <user/types.h>



void layer3_init(void){
}



void layer3_poll(void){
	u8 buffer[64];
	network_packet_t packet={
		.buffer_length=64,
		.buffer=buffer
	};
	if (!network_poll(&packet)||packet.protocol!=LAYER3_PROTOCOL||!packet.buffer_length){
		return;
	}
	_Bool is_response=buffer[0]&1;
	switch (buffer[0]>>1){
		case LAYER3_PACKET_TYPE_PING_PONG:
			if (is_response){
				printf("[layer3] Pong from %X:%X:%X:%X:%X:%X\n",packet.address[0],packet.address[1],packet.address[2],packet.address[3],packet.address[4],packet.address[5]);
			}
			else{
				buffer[0]|=1;
				packet.buffer_length=1;
				network_send(&packet);
			}
			break;
		default:
			printf("[layer3] Unknown packet type '%X/%u' received from %X:%X:%X:%X:%X:%X",buffer[0]>>1,is_response,packet.address[0],packet.address[1],packet.address[2],packet.address[3],packet.address[4],packet.address[5]);
			break;
	}
}
