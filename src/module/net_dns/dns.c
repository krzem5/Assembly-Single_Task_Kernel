#include <kernel/log/log.h>
#include <kernel/memory/amm.h>
#include <kernel/memory/smm.h>
#include <kernel/mp/thread.h>
#include <kernel/socket/socket.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#include <kernel/vfs/node.h>
#include <kernel/vfs/vfs.h>
#include <net/dns.h>
#include <net/info.h>
#include <net/ip4.h>
#include <net/udp.h>
#define KERNEL_LOG_NAME "net_dns"



static vfs_node_t* _net_dns_socket=NULL;
static KERNEL_ATOMIC u16 _net_dns_request_id=0;



static u32 _encode_name(const char* name,u8* out,u32 max_size){
	u32 i=0;
	u32 j=0;
	do{
		j=0;
		for (;name[j]&&name[j]!='.';j++);
		if (j>255){
			ERROR("Domain label too long");
			return 0;
		}
		if (i+j+1>max_size){
			return 0;
		}
		out[i]=j;
		memcpy(out+i+1,name,j);
		i+=j+1;
		name+=j;
		if (name[0]=='.'){
			name++;
		}
	} while (j);
	return i;
}



static void _rx_thread(void){
	while (1){
		net_udp_socket_packet_t* packet=socket_pop_packet(_net_dns_socket,0);
		ERROR("PACKET: DNS (%u)",packet->length);
		amm_dealloc(packet);
	}
}



void net_dns_init(void){
	LOG("Initializing DNS resolver...");
	SMM_TEMPORARY_STRING name=smm_alloc("dns_socket",0);
	_net_dns_socket=socket_create(vfs_lookup(NULL,"/",0,0,0),name,SOCKET_DOMAIN_INET,SOCKET_TYPE_DGRAM,SOCKET_PROTOCOL_UDP);
	net_udp_address_t local_address={
		0x00000000,
		53
	};
	if (!socket_bind(_net_dns_socket,&local_address,sizeof(net_udp_address_t))){
		ERROR("Failed to bind DNS client socket");
		return;
	}
	thread_new_kernel_thread(NULL,"net-dns-rx-thread",_rx_thread,0x200000,0);
}



KERNEL_PUBLIC net_ip4_address_t net_dns_lookup_name(const char* name,_Bool nonblocking){
	// same cache as ARP, key=length<<32|hash (computed by string_t) + override entries if key matches but content does not
	if (nonblocking){
		return 0;
	}
	_net_dns_request_id++;
	u8 buffer[sizeof(net_udp_socket_packet_t)+512];
	net_udp_socket_packet_t* udp_packet=(net_udp_socket_packet_t*)buffer;
	udp_packet->src_address=0x0a00020f;
	udp_packet->dst_address=0x0a000203;
	udp_packet->src_port=53;
	udp_packet->dst_port=53;
	net_dns_packet_t* header=(net_dns_packet_t*)(udp_packet->data);
	header->id=__builtin_bswap16(_net_dns_request_id);
	header->flags=__builtin_bswap16(NET_DNS_OPCODE_QUERY);
	header->qdcount=__builtin_bswap16(1);
	header->ancount=0;
	header->nscount=0;
	header->arcount=0;
	u16 offset=_encode_name(name,header->data,512-sizeof(net_dns_packet_t));
	if (!offset||512-sizeof(net_dns_packet_t)-offset<sizeof(net_dns_packet_question_t)){
		return 0;
	}
	net_dns_packet_question_t* question=(net_dns_packet_question_t*)(header->data+offset);
	question->qtype=__builtin_bswap16(NET_DNS_QTYPE_ALL);
	question->qclass=__builtin_bswap16(NET_DNS_QCLASS_IN);
	udp_packet->length=sizeof(net_dns_packet_t)+offset+sizeof(net_dns_packet_question_t);
	socket_push_packet(_net_dns_socket,udp_packet,sizeof(net_udp_socket_packet_t)+udp_packet->length);
	return 0;
}
