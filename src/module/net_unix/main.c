#include <kernel/module/module.h>
#include <net/unix.h>
//////////////
#include <kernel/socket/socket.h>



static _Bool _init(module_t* module){
	net_unix_init();
	vfs_node_t* A=socket_create(SOCKET_DOMAIN_UNIX,SOCKET_TYPE_DGRAM,SOCKET_PROTOCOL_NONE);
	vfs_node_t* B=socket_create(SOCKET_DOMAIN_UNIX,SOCKET_TYPE_DGRAM,SOCKET_PROTOCOL_NONE);
	net_unix_address_t address={
		"/test_unix_socket"
	};
	socket_bind(B,&address,sizeof(net_unix_address_t));
	socket_connect(A,&address,sizeof(net_unix_address_t));
	return 1;
}



static void _deinit(module_t* module){
	return;
}



MODULE_DECLARE(
	_init,
	_deinit,
	0
);
