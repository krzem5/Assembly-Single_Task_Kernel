#ifndef _NET_DNS_H_
#define _NET_DNS_H_ 1
#include <kernel/types.h>
#include <net/ip4.h>



// DNS flags
#define NET_DNS_FLAG_QR 1
#define NET_DNS_FLAG_AA 32
#define NET_DNS_FLAG_TC 64
#define NET_DNS_FLAG_RD 128
#define NET_DNS_FLAG_RA 256

// DNS opcode
#define NET_DNS_OPCODE_MASK 0x1e
#define NET_DNS_OPCODE_QUERY 0
#define NET_DNS_OPCODE_IQUERY 2
#define NET_DNS_OPCODE_STATUS 4

// DNS return code
#define NET_DNS_RCODE_MASK 0xf000
#define NET_DNS_RCODE_NO_ERROR 0
#define NET_DNS_RCODE_FORMAT_ERROR 4096
#define NET_DNS_RCODE_SERVER_FAILURE 8192
#define NET_DNS_RCODE_NAME_ERROR 12288
#define NET_DNS_RCODE_NOT_IMPLEMENTED 16384
#define NET_DNS_RCODE_REFUSED 20480

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



void net_dns_init(void);



net_ip4_address_t net_dns_lookup_name(const char* name,_Bool nonblocking);



#endif
