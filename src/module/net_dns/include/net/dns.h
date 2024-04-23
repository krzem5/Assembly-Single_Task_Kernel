#ifndef _NET_DNS_H_
#define _NET_DNS_H_ 1
#include <kernel/memory/smm.h>
#include <kernel/tree/rb_tree.h>
#include <kernel/types.h>
#include <net/ip4.h>



// DNS flags
#define NET_DNS_FLAG_QR 0x8000
#define NET_DNS_FLAG_AA 0x0400
#define NET_DNS_FLAG_TC 0x0200
#define NET_DNS_FLAG_RD 0x0100
#define NET_DNS_FLAG_RA 0x0080

// DNS opcode
#define NET_DNS_OPCODE_MASK 0x7800
#define NET_DNS_OPCODE_QUERY 0x0000
#define NET_DNS_OPCODE_IQUERY 0x0800
#define NET_DNS_OPCODE_STATUS 0x1000

// DNS return code
#define NET_DNS_RCODE_MASK 0x000f
#define NET_DNS_RCODE_NO_ERROR 0x0000
#define NET_DNS_RCODE_FORMAT_ERROR 0x0001
#define NET_DNS_RCODE_SERVER_FAILURE 0x0002
#define NET_DNS_RCODE_NAME_ERROR 0x0003
#define NET_DNS_RCODE_NOT_IMPLEMENTED 0x0004
#define NET_DNS_RCODE_REFUSED 0x0005

// DNS types
#define NET_DNS_TYPE_A 1
#define NET_DNS_TYPE_NS 2
#define NET_DNS_TYPE_MD 3
#define NET_DNS_TYPE_MF 4
#define NET_DNS_TYPE_CNAME 5
#define NET_DNS_TYPE_SOA 6
#define NET_DNS_TYPE_MB 7
#define NET_DNS_TYPE_MG 8
#define NET_DNS_TYPE_MR 9
#define NET_DNS_TYPE_NULL 10
#define NET_DNS_TYPE_WKS 11
#define NET_DNS_TYPE_PTR 12
#define NET_DNS_TYPE_HINFO 13
#define NET_DNS_TYPE_MINFO 14
#define NET_DNS_TYPE_MX 15
#define NET_DNS_TYPE_TXT 16

// DNS question-specific types
#define NET_DNS_QTYPE_AXFR 252
#define NET_DNS_QTYPE_MAILB 253
#define NET_DNS_QTYPE_MAILA 254
#define NET_DNS_QTYPE_ALL 255

// DNS question classes
#define NET_DNS_QCLASS_IN 1
#define NET_DNS_QCLASS_CS 2
#define NET_DNS_QCLASS_CH 3
#define NET_DNS_QCLASS_HS 4



typedef struct KERNEL_PACKED _NET_DNS_PACKET{
	u16 id;
	u16 flags;
	u16 qdcount;
	u16 ancount;
	u16 nscount;
	u16 arcount;
	u8 data[];
} net_dns_packet_t;



typedef struct KERNEL_PACKED _NET_DNS_PACKET_QUESTION{
	u16 qtype;
	u16 qclass;
} net_dns_packet_question_t;



typedef struct KERNEL_PACKED _NET_DNS_PACKET_RESOURCE_RECORD{
	u16 type;
	u16 class;
	u32 ttl;
	u16 rdlength;
	u8 rdata[];
} net_dns_packet_resource_record_t;



typedef struct _NET_DNS_CACHE_ENTRY{
	rb_tree_node_t rb_node;
	string_t* name;
	u64 last_valid_time;
	net_ip4_address_t address;
} net_dns_cache_entry_t;



typedef struct _NET_DNS_REQUEST{
	rb_tree_node_t rb_node;
	string_t* name;
	net_ip4_address_t address;
	u32 cache_duration;
} net_dns_request_t;



net_ip4_address_t net_dns_lookup_name(const char* name,bool nonblocking);



#endif
