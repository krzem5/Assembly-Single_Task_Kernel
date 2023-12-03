#include <kernel/log/log.h>
#include <kernel/memory/smm.h>
#include <kernel/mp/process.h>
#include <kernel/mp/thread.h>
#include <kernel/network/layer1.h>
#include <kernel/scheduler/scheduler.h>
#include <kernel/socket/socket.h>
#include <kernel/util/util.h>
#include <kernel/vfs/node.h>
#include <kernel/vfs/vfs.h>
#include <net/dhcp.h>
#include <net/udp.h>
#define KERNEL_LOG_NAME "net_dhcp"



static vfs_node_t* _net_dhcp_socket=NULL;



static void _rx_thread(void){
	u8 buffer[4096];
	while (1){
		u64 size=vfs_node_read(_net_dhcp_socket,0,buffer,4096,0);
		if (size<sizeof(net_dhcp_packet_t)){
			continue;
		}
		net_dhcp_packet_t* dhcp_packet=(net_dhcp_packet_t*)buffer;
		WARN("PACKET [%I %I %I %I]",__builtin_bswap32(dhcp_packet->ciaddr),__builtin_bswap32(dhcp_packet->yiaddr),__builtin_bswap32(dhcp_packet->siaddr),__builtin_bswap32(dhcp_packet->giaddr));
	}
}



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
	thread_new_kernel_thread(NULL,_rx_thread,0x200000,0);
	u8 buffer[sizeof(net_dhcp_packet_t)+4];
	memset(buffer,0,sizeof(net_dhcp_packet_t)+4);
	net_dhcp_packet_t* packet=(net_dhcp_packet_t*)buffer;
	packet->op=1;
	packet->htype=1;
	packet->hlen=6;
	packet->hops=0;
	packet->xid=0xa5a5a5a5; // random
	packet->secs=0;
	packet->flags=0;
	packet->ciaddr=0;
	packet->yiaddr=0;
	packet->siaddr=0;
	packet->giaddr=0;
	memcpy(packet->chaddr,network_layer1_device->mac_address,sizeof(mac_address_t));
	packet->cookie=__builtin_bswap32(NET_DHCP_COOKIE);
	packet->options[0]=53;
	packet->options[1]=1;
	packet->options[2]=1;
	packet->options[3]=255;
	vfs_node_write(_net_dhcp_socket,0,buffer,sizeof(net_dhcp_packet_t)+4,0);
}
