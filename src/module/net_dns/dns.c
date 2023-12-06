#include <kernel/log/log.h>
#include <kernel/memory/amm.h>
#include <kernel/mp/thread.h>
#include <kernel/socket/socket.h>
#include <kernel/types.h>
#include <kernel/vfs/node.h>
#include <kernel/vfs/vfs.h>
#include <net/info.h>
#include <net/ip4.h>
#include <net/udp.h>
#define KERNEL_LOG_NAME "net_dns"



static vfs_node_t* _net_dns_socket=NULL;



static void _rx_thread(void){
	while (1){
		net_udp_socket_packet_t* packet=socket_get_packet(_net_dns_socket,0);
		ERROR("%u",packet->length);
		amm_dealloc(packet);
	}
}



void net_dns_init(void){
	LOG("Initializing DNS resolver...");
	SMM_TEMPORARY_STRING name=smm_alloc("dns_socket",0);
	_net_dns_socket=socket_create(vfs_lookup(NULL,"/",0,0,0),name,SOCKET_DOMAIN_INET,SOCKET_TYPE_DGRAM,SOCKET_PROTOCOL_UDP);
	thread_new_kernel_thread(NULL,_rx_thread,0x200000,0);
}



KERNEL_PUBLIC net_ip4_address_t net_dns_lookup_name(const char* name){
	return 0;
}
