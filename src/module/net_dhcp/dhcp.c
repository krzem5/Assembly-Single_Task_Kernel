#include <kernel/log/log.h>
#include <kernel/memory/smm.h>
#include <kernel/socket/socket.h>
#include <kernel/vfs/node.h>
#include <kernel/vfs/vfs.h>
#include <net/dhcp.h>
#include <net/udp.h>
#define KERNEL_LOG_NAME "net_dhcp"



static vfs_node_t* _net_dhcp_socket=NULL;



void net_dhcp_init(void){
	LOG("Initializing DHCP client...");
	SMM_TEMPORARY_STRING name=smm_alloc("dhcp_socket",0);
	_net_dhcp_socket=socket_create(vfs_lookup(NULL,"/",0,0,0),name,SOCKET_DOMAIN_INET,SOCKET_TYPE_DGRAM,SOCKET_PROTOCOL_UDP);
	net_udp_address_t local_address={
		0x00000000,
		68
	};
	if (!socket_bind(_net_dhcp_socket,&local_address,sizeof(net_udp_address_t))){
		ERROR("Failed to bind DHCP client socket");
		return;
	}
	net_udp_address_t remote_address={
		0xffffffff,
		67
	};
	if (!socket_connect(_net_dhcp_socket,&remote_address,sizeof(net_udp_address_t))){
		ERROR("Failed to connect DHCP client socket");
		return;
	}
}
