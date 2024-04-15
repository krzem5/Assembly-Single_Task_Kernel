#include <kernel/acl/acl.h>
#include <kernel/handle/handle.h>
#include <kernel/keyring/keyring.h>
#include <kernel/lock/spinlock.h>
#include <kernel/log/log.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/smm.h>
#include <kernel/notification/notification.h>
#include <kernel/rsa/rsa.h>
#include <kernel/types.h>
#define KERNEL_LOG_NAME "keyring"



static volatile u8 KERNEL_EARLY_READ __kernel_module_key_exponent[1024];
static volatile u8 KERNEL_EARLY_READ __kernel_module_key_modulus[1024];
static volatile u32 KERNEL_EARLY_READ __kernel_module_key_modulus_bit_length;
static volatile u8 KERNEL_EARLY_READ __kernel_user_key_exponent[1024];
static volatile u8 KERNEL_EARLY_READ __kernel_user_key_modulus[1024];
static volatile u32 KERNEL_EARLY_READ __kernel_user_key_modulus_bit_length;

static omm_allocator_t* KERNEL_INIT_WRITE _keyring_allocator=NULL;
static omm_allocator_t* KERNEL_INIT_WRITE _keyring_key_allocator=NULL;
static handle_type_t KERNEL_INIT_WRITE _keyring_handle_type=0;
static notification_dispatcher_t _keyring_notification_dispatcher;
static spinlock_t _keyring_creation_lock;

KERNEL_PUBLIC keyring_t* keyring_module_signature=NULL;
KERNEL_PUBLIC keyring_t* keyring_user_signature=NULL;



static void _keyring_handle_destructor(handle_t* handle){
	keyring_t* keyring=handle->object;
	ERROR("Delete keyring '%s'",keyring->name->data);
}



KERNEL_INIT(){
	LOG("Initializing keyrings...");
	_keyring_allocator=omm_init("keyring",sizeof(keyring_t),8,2,pmm_alloc_counter("omm_keyring"));
	spinlock_init(&(_keyring_allocator->lock));
	_keyring_key_allocator=omm_init("keyring_key",sizeof(keyring_key_t),8,2,pmm_alloc_counter("omm_keyring_key"));
	spinlock_init(&(_keyring_key_allocator->lock));
	_keyring_handle_type=handle_alloc("keyring",_keyring_handle_destructor);
	notification_dispatcher_init(&_keyring_notification_dispatcher);
	spinlock_init(&_keyring_creation_lock);
	INFO("Creating module signature keyring...");
	keyring_module_signature=keyring_create("module-signature");
	keyring_key_t* key=keyring_key_create(keyring_module_signature,"builtin-module");
	key->type=KEYRING_KEY_TYPE_RSA;
	key->flags|=KEYRING_KEY_FLAG_VIRTUAL;
	rsa_state_init((const void*)__kernel_module_key_modulus,__kernel_module_key_modulus_bit_length,&(key->data.rsa.state));
	key->data.rsa.state.public_key=rsa_number_create_from_bytes(&(key->data.rsa.state),(const void*)__kernel_module_key_exponent,1024/sizeof(u32));
	INFO("Creating user signature keyring...");
	keyring_user_signature=keyring_create("user-signature");
	key=keyring_key_create(keyring_user_signature,"builtin-user");
	key->type=KEYRING_KEY_TYPE_RSA;
	key->flags|=KEYRING_KEY_FLAG_VIRTUAL;
	rsa_state_init((const void*)__kernel_user_key_modulus,__kernel_user_key_modulus_bit_length,&(key->data.rsa.state));
	key->data.rsa.state.public_key=rsa_number_create_from_bytes(&(key->data.rsa.state),(const void*)__kernel_user_key_exponent,1024/sizeof(u32));
}



KERNEL_PUBLIC keyring_t* keyring_create(const char* name){
	string_t* name_string=smm_alloc(name,0);
	spinlock_acquire_exclusive(&_keyring_creation_lock);
	HANDLE_FOREACH(_keyring_handle_type){
		keyring_t* keyring=handle->object;
		if (smm_equal(keyring->name,name_string)){
			spinlock_release_exclusive(&_keyring_creation_lock);
			smm_dealloc(name_string);
			return keyring;
		}
	}
	keyring_t* out=omm_alloc(_keyring_allocator);
	out->name=name_string;
	handle_new(out,_keyring_handle_type,&(out->handle));
	out->handle.acl=acl_create();
	spinlock_init(&(out->lock));
	out->head=NULL;
	spinlock_release_exclusive(&_keyring_creation_lock);
	handle_finish_setup(&(out->handle));
	notification_dispatcher_dispatch(&_keyring_notification_dispatcher,out,NOTIFICATION_TYPE_KEYRING_UPDATE);
	return out;
}



KERNEL_PUBLIC void keyring_delete(keyring_t* keyring);



KERNEL_PUBLIC keyring_key_t* keyring_search(keyring_t* keyring,const char* name,u32 flags){
	SMM_TEMPORARY_STRING name_string=smm_alloc(name,0);
	spinlock_acquire_shared(&(keyring->lock));
	keyring_key_t* key=keyring->head;
	for (;key&&!smm_equal(key->name,name_string);key=key->next);
	spinlock_release_shared(&(keyring->lock));
	return key;
}



KERNEL_PUBLIC keyring_key_t* keyring_key_create(keyring_t* keyring,const char* name){
	string_t* name_string=smm_alloc(name,0);
	spinlock_acquire_exclusive(&(keyring->lock));
	for (keyring_key_t* key=keyring->head;key;key=key->next){
		if (smm_equal(key->name,name_string)){
			spinlock_release_exclusive(&(keyring->lock));
			smm_dealloc(name_string);
			return NULL;
		}
	}
	keyring_key_t* out=omm_alloc(_keyring_key_allocator);
	out->keyring=keyring;
	out->name=name_string;
	out->type=KEYRING_KEY_TYPE_NONE;
	out->flags=0;
	spinlock_init(&(out->lock));
	out->prev=NULL;
	out->next=keyring->head;
	if (keyring->head){
		keyring->head->prev=out;
	}
	else{
		keyring->head=out;
	}
	spinlock_release_exclusive(&(keyring->lock));
	notification_dispatcher_dispatch(&_keyring_notification_dispatcher,keyring,NOTIFICATION_TYPE_KEYRING_UPDATE);
	return out;
}



KERNEL_PUBLIC void keyring_key_delete(keyring_key_t* key);



KERNEL_PUBLIC _Bool keyring_key_process_rsa(keyring_key_t* key,rsa_number_t* in,rsa_number_t* out){
	spinlock_acquire_exclusive(&(key->lock));
	_Bool ret=0;
	if (key->type==KEYRING_KEY_TYPE_RSA){
		rsa_state_process(&(key->data.rsa.state),in,RSA_PUBLIC_KEY,out);
		ret=1;
	}
	spinlock_release_exclusive(&(key->lock));
	return ret;
}



KERNEL_PUBLIC void keyring_register_notification_listener(notification_listener_callback_t listener_callback){
	notification_dispatcher_add_listener(&_keyring_notification_dispatcher,listener_callback);
	HANDLE_FOREACH(_keyring_handle_type){
		listener_callback(handle->object,NOTIFICATION_TYPE_KEYRING_UPDATE);
	}
}



KERNEL_PUBLIC void keyring_unregister_notification_listener(notification_listener_callback_t listener_callback){
	notification_dispatcher_add_listener(&_keyring_notification_dispatcher,listener_callback);
}
