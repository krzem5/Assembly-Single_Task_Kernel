#ifndef _KERNEL_KEYRING_KEYRING_H_
#define _KERNEL_KEYRING_KEYRING_H_ 1
#include <kernel/handle/handle.h>
#include <kernel/lock/rwlock.h>
#include <kernel/memory/smm.h>
#include <kernel/notification/notification.h>
#include <kernel/rsa/rsa.h>
#include <kernel/types.h>



#define KEYRING_KEY_TYPE_NONE 0
#define KEYRING_KEY_TYPE_RAW 1
#define KEYRING_KEY_TYPE_RSA 2

#define KEYRING_KEY_FLAG_VIRTUAL 1

#define KEYRING_SEARCH_FLAG_BYPASS_ACL 1



typedef struct _KEYRING_KEY{
	struct _KEYRING* keyring;
	struct _KEYRING_KEY* prev;
	struct _KEYRING_KEY* next;
	string_t* name;
	u16 type;
	u16 flags;
	rwlock_t lock;
	union{
		struct{
			void* payload;
			u64 payload_length;
		} raw;
		struct{
			rsa_state_t state;
		} rsa;
	} data;
} keyring_key_t;



typedef struct _KEYRING{
	string_t* name;
	handle_t handle;
	rwlock_t lock;
	keyring_key_t* head;
} keyring_t;



extern handle_type_t keyring_handle_type;
extern keyring_t* keyring_module_signature;
extern keyring_t* keyring_user_signature;
extern notification2_dispatcher_t keyring_notification_dispatcher;



keyring_t* keyring_create(const char* name);



void keyring_delete(keyring_t* keyring);



keyring_key_t* keyring_search(keyring_t* keyring,const char* name,u32 flags);



keyring_key_t* keyring_key_create(keyring_t* keyring,const char* name);



void keyring_key_delete(keyring_key_t* key);



void keyring_key_update(keyring_key_t* key);



bool keyring_key_process_rsa(keyring_key_t* key,rsa_number_t* in,rsa_number_t* out);



#endif
