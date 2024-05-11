#include <kernel/lock/rwlock.h>
#include <kernel/log/log.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/socket/port.h>
#include <kernel/socket/socket.h>
#include <kernel/types.h>
#define KERNEL_LOG_NAME "socket_port"



static rwlock_t _socket_port_lock;
static socket_vfs_node_t** KERNEL_INIT_WRITE _socket_ports=NULL;



KERNEL_INIT(){
	LOG("Initializing socket ports...");
	rwlock_init(&_socket_port_lock);
	_socket_ports=(void*)(pmm_alloc(pmm_align_up_address((SOCKET_PORT_MAX+1)*sizeof(socket_vfs_node_t*))>>PAGE_SIZE_SHIFT,pmm_alloc_counter("kernel.socket.port"),0)+VMM_HIGHER_HALF_ADDRESS_OFFSET);
}



KERNEL_PUBLIC socket_vfs_node_t* socket_port_get(socket_port_t port){
	return _socket_ports[port];
}



KERNEL_PUBLIC bool socket_port_reserve(socket_vfs_node_t* socket,socket_port_t port){
	rwlock_acquire_write(&_socket_port_lock);
	bool out=!_socket_ports[port];
	if (!_socket_ports[port]){
		_socket_ports[port]=socket;
	}
	rwlock_release_write(&_socket_port_lock);
	return out;
}
