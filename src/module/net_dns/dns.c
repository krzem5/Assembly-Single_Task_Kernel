#include <kernel/clock/clock.h>
#include <kernel/lock/rwlock.h>
#include <kernel/log/log.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/smm.h>
#include <kernel/module/module.h>
#include <kernel/mp/event.h>
#include <kernel/mp/thread.h>
#include <kernel/socket/socket.h>
#include <kernel/timer/timer.h>
#include <kernel/tree/rb_tree.h>
#include <kernel/types.h>
#include <kernel/util/memory.h>
#include <kernel/vfs/node.h>
#include <net/dns.h>
#include <net/info.h>
#include <net/ip4.h>
#include <net/udp.h>
#define KERNEL_LOG_NAME "net_dns"



#define DNS_TIMEOUT_NS 1000000000 // 1 s
#define DNS_CACHE_CLEAR_INTERVAL_NS 1800000000000 // 30 min



static omm_allocator_t* KERNEL_INIT_WRITE _net_dns_cache_entry_allocator=NULL;
static omm_allocator_t* KERNEL_INIT_WRITE _net_dns_request_allocator=NULL;
static event_t* KERNEL_INIT_WRITE _net_dns_cache_resolution_event=NULL;
static rwlock_t _net_dns_cache_lock;
static rb_tree_t _net_dns_cache_address_tree;
static rwlock_t _net_dns_request_tree_lock;
static rb_tree_t _net_dns_request_tree;
static KERNEL_ATOMIC u16 _net_dns_request_id=0;
static vfs_node_t* KERNEL_INIT_WRITE _net_dns_socket=NULL;



static u32 _skip_name(const u8* data,u32 max_size){
	u32 i=0;
	while (i<max_size){
		u8 j=data[i];
		i++;
		if (!j){
			return i;
		}
		i+=((j>>6)==3?1:j);
	}
	return 0;
}



static u32 _decode_name(const u8* data,u32 offset,u32 max_size,char* out,u32 out_index,u32 out_length){
	while (offset<max_size){
		u8 i=data[offset];
		offset++;
		if (!i){
			return offset;
		}
		if ((i>>6)==3){
			u16 j=((i&0x3f)<<8)|data[offset];
			offset++;
			if (offset>max_size||j<sizeof(net_dns_packet_t)){
				return 0;
			}
			return (_decode_name(data,j-sizeof(net_dns_packet_t),max_size,out,out_index,out_length)?offset:0);
		}
		if (i+out_index+1>out_length||i+offset>max_size){
			return 0;
		}
		if (out_index){
			out[out_index-1]='.';
		}
		mem_copy(out+out_index,data+offset,i);
		out_index+=i;
		out[out_index]=0;
		out_index++;
		offset+=i;
	}
	return 0;
}



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
		mem_copy(out+i+1,name,j);
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
		socket_packet_t* socket_packet=socket_pop_packet(_net_dns_socket,0);
		net_udp_socket_packet_t* packet=socket_packet->data;
		if (packet->length<sizeof(net_dns_packet_t)){
			goto _cleanup;
		}
		net_dns_packet_t* dns_packet=(net_dns_packet_t*)(packet->data);
		u16 flags=__builtin_bswap16(dns_packet->flags);
		if (!(flags&NET_DNS_FLAG_QR)||!dns_packet->ancount||(flags&NET_DNS_RCODE_MASK)!=NET_DNS_RCODE_NO_ERROR){
			goto _cleanup;
		}
		rwlock_acquire_write(&_net_dns_request_tree_lock);
		net_dns_request_t* request=(net_dns_request_t*)rb_tree_lookup_node(&_net_dns_request_tree,__builtin_bswap16(dns_packet->id));
		if (!request){
			goto _cleanup_and_release_lock;
		}
		u32 offset=0;
		for (u16 i=0;i<__builtin_bswap16(dns_packet->qdcount);i++){
			offset=_skip_name(dns_packet->data+offset,packet->length-sizeof(net_dns_packet_t)-offset);
			if (!offset){
				goto _cleanup_and_release_lock;
			}
			offset+=sizeof(net_dns_packet_question_t);
			if (offset+sizeof(net_dns_packet_t)>packet->length){
				goto _cleanup_and_release_lock;
			}
		}
		char buffer[256];
		offset=_decode_name(dns_packet->data,offset,packet->length-sizeof(net_dns_packet_t),buffer,0,256);
		if (!offset||offset+sizeof(net_dns_packet_resource_record_t)+sizeof(net_dns_packet_t)>packet->length){
			goto _cleanup_and_release_lock;
		}
		net_dns_packet_resource_record_t* resouce_record=(net_dns_packet_resource_record_t*)(dns_packet->data+offset);
		offset+=sizeof(net_dns_packet_resource_record_t)+__builtin_bswap16(resouce_record->rdlength);
		if (offset+sizeof(net_dns_packet_t)>packet->length||resouce_record->type!=__builtin_bswap16(NET_DNS_TYPE_A)||resouce_record->class!=__builtin_bswap16(NET_DNS_QCLASS_IN)||resouce_record->rdlength!=__builtin_bswap16(sizeof(net_ip4_address_t))){
			goto _cleanup_and_release_lock;
		}
		request->address=__builtin_bswap32(*((net_ip4_address_t*)(resouce_record->rdata)));
		request->cache_duration=__builtin_bswap32(resouce_record->ttl);
		INFO("DNS resolution: %s -> %I",buffer,request->address);
		event_dispatch(_net_dns_cache_resolution_event,EVENT_DISPATCH_FLAG_DISPATCH_ALL);
_cleanup_and_release_lock:
		rwlock_release_write(&_net_dns_request_tree_lock);
_cleanup:
		socket_dealloc_packet(socket_packet);
	}
}



static void _cache_cleanup_thread(void){
	timer_t* timer=timer_create("net.dns.cache_cleanup",DNS_CACHE_CLEAR_INTERVAL_NS,TIMER_COUNT_INFINITE);
	while (1){
		event_await(timer->event,0);
		INFO("Cleaning-up expired cache...");
		rwlock_acquire_write(&_net_dns_cache_lock);
		u64 time=clock_get_time();
		for (rb_tree_node_t* rb_node=rb_tree_iter_start(&_net_dns_cache_address_tree);rb_node;){
			net_dns_cache_entry_t* cache_entry=(net_dns_cache_entry_t*)rb_node;
			rb_node=rb_tree_iter_next(&_net_dns_cache_address_tree,rb_node);
			if (cache_entry->last_valid_time>=time){
				continue;
			}
			INFO("Deleting entry for '%s'...",cache_entry->name->data);
			rb_tree_remove_node(&_net_dns_cache_address_tree,&(cache_entry->rb_node));
			smm_dealloc(cache_entry->name);
			omm_dealloc(_net_dns_cache_entry_allocator,cache_entry);
		}
		rwlock_release_write(&_net_dns_cache_lock);
	}
}



MODULE_INIT(){
	LOG("Initializing DNS resolver...");
	_net_dns_cache_entry_allocator=omm_init("net.dns.cache_entry",sizeof(net_dns_cache_entry_t),8,4);
	rwlock_init(&(_net_dns_cache_entry_allocator->lock));
	_net_dns_request_allocator=omm_init("net.dns.request",sizeof(net_dns_request_t),8,4);
	rwlock_init(&(_net_dns_request_allocator->lock));
	_net_dns_cache_resolution_event=event_create("net.dns.resolution",NULL);
	rwlock_init(&_net_dns_cache_lock);
	rb_tree_init(&_net_dns_cache_address_tree);
	rwlock_init(&_net_dns_request_tree_lock);
	rb_tree_init(&_net_dns_request_tree);
	_net_dns_socket=socket_create(SOCKET_DOMAIN_INET,SOCKET_TYPE_DGRAM,SOCKET_PROTOCOL_UDP);
	net_udp_address_t local_address={
		NET_UDP_ADDRESS_TYPE_IP4,
		53,
		{
			.ip4=0x00000000,
		}
	};
	if (!socket_bind(_net_dns_socket,&local_address,sizeof(net_udp_address_t))){
		ERROR("Failed to bind DNS client socket");
		return;
	}
	thread_create_kernel_thread(NULL,"net.dns.rx",_rx_thread,0);
	thread_create_kernel_thread(NULL,"net.dns.cache_cleanup",_cache_cleanup_thread,0);
}



KERNEL_PUBLIC net_ip4_address_t net_dns_lookup_name(const char* name,bool nonblocking){
	string_t* name_string=smm_alloc(name,0);
	u64 key=(((u64)(name_string->length))<<32)|name_string->hash;
	rwlock_acquire_write(&_net_dns_cache_lock);
	net_dns_cache_entry_t* cache_entry=(net_dns_cache_entry_t*)rb_tree_lookup_node(&_net_dns_cache_address_tree,key);
	if (cache_entry){
		if (cache_entry->last_valid_time<clock_get_time()){
			goto _invalid_cache_entry;
		}
		for (u32 i=0;i<name_string->length;i++){
			if (cache_entry->name->data[i]!=name[i]){
				goto _invalid_cache_entry;
			}
		}
		net_ip4_address_t out=cache_entry->address;
		rwlock_release_write(&_net_dns_cache_lock);
		return out;
_invalid_cache_entry:
		rb_tree_remove_node(&_net_dns_cache_address_tree,&(cache_entry->rb_node));
		smm_dealloc(cache_entry->name);
		omm_dealloc(_net_dns_cache_entry_allocator,cache_entry);
	}
	rwlock_release_write(&_net_dns_cache_lock);
	const net_info_address_list_entry_t* dns_entry=net_info_get_dns_entries();
	if (nonblocking||!dns_entry){
		return 0;
	}
	rwlock_acquire_write(&_net_dns_request_tree_lock);
	u16 request_id=_net_dns_request_id;
	_net_dns_request_id++;
	net_dns_request_t* request=(net_dns_request_t*)rb_tree_lookup_node(&_net_dns_request_tree,request_id);
	if (request){
		_net_dns_request_id--;
		rwlock_release_write(&_net_dns_request_tree_lock);
		return 0;
	}
	request=omm_alloc(_net_dns_request_allocator);
	request->rb_node.key=request_id;
	request->name=name_string;
	request->address=0;
	rb_tree_insert_node(&_net_dns_request_tree,&(request->rb_node));
	rwlock_release_write(&_net_dns_request_tree_lock);
	INFO("DNS request: %s",name);
	u8 buffer[sizeof(net_udp_socket_packet_t)+512];
	net_udp_socket_packet_t* udp_packet=(net_udp_socket_packet_t*)buffer;
	udp_packet->src_address.type=NET_UDP_ADDRESS_TYPE_IP4;
	udp_packet->src_address.port=53;
	udp_packet->src_address.address.ip4=net_info_get_address();
	udp_packet->dst_address.type=NET_UDP_ADDRESS_TYPE_IP4;
	udp_packet->dst_address.port=53;
	udp_packet->dst_address.address.ip4=dns_entry->address;
	net_dns_packet_t* header=(net_dns_packet_t*)(udp_packet->data);
	header->id=__builtin_bswap16(request_id);
	header->flags=__builtin_bswap16(NET_DNS_OPCODE_QUERY);
	header->qdcount=__builtin_bswap16(1);
	header->ancount=0;
	header->nscount=0;
	header->arcount=0;
	u16 offset=_encode_name(name,header->data,512-sizeof(net_dns_packet_t));
	if (!offset||512-sizeof(net_dns_packet_t)-offset<sizeof(net_dns_packet_question_t)){
		WARN("Request too large for DNS over UDP");
		return 0;
	}
	net_dns_packet_question_t* question=(net_dns_packet_question_t*)(header->data+offset);
	question->qtype=__builtin_bswap16(NET_DNS_TYPE_A);
	question->qclass=__builtin_bswap16(NET_DNS_QCLASS_IN);
	udp_packet->length=sizeof(net_dns_packet_t)+offset+sizeof(net_dns_packet_question_t);
	socket_push_packet(_net_dns_socket,udp_packet,sizeof(net_udp_socket_packet_t)+udp_packet->length);
	timer_t* timer=timer_create("net.dns.query.timeout",DNS_TIMEOUT_NS,1);
	event_t* events[2]={
		timer->event,
		_net_dns_cache_resolution_event
	};
	while (event_await_multiple(events,2)&&!request->address);
	timer_delete(timer);
	rwlock_acquire_write(&_net_dns_request_tree_lock);
	rb_tree_remove_node(&_net_dns_request_tree,&(request->rb_node));
	rwlock_release_write(&_net_dns_request_tree_lock);
	net_ip4_address_t out=request->address;
	if (!out||!request->cache_duration){
		smm_dealloc(request->name);
	}
	else{
		rwlock_acquire_write(&_net_dns_cache_lock);
		net_dns_cache_entry_t* cache_entry=(net_dns_cache_entry_t*)rb_tree_lookup_node(&_net_dns_cache_address_tree,key);
		if (cache_entry){
			smm_dealloc(cache_entry->name);
		}
		else{
			cache_entry=omm_alloc(_net_dns_cache_entry_allocator);
			cache_entry->rb_node.key=key;
			rb_tree_insert_node(&_net_dns_cache_address_tree,&(cache_entry->rb_node));
		}
		cache_entry->name=request->name;
		cache_entry->last_valid_time=clock_get_time()+request->cache_duration*1000000000ull;
		cache_entry->address=request->address;
		rwlock_release_write(&_net_dns_cache_lock);
	}
	omm_dealloc(_net_dns_request_allocator,request);
	return out;
}
