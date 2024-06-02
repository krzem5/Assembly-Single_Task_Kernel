#include <kernel/acl/acl.h>
#include <kernel/handle/handle.h>
#include <kernel/keyring/keyring.h>
#include <kernel/lock/rwlock.h>
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
static rwlock_t _keyring_creation_lock;

KERNEL_PUBLIC handle_type_t KERNEL_INIT_WRITE keyring_handle_type=0;
KERNEL_PUBLIC keyring_t* KERNEL_INIT_WRITE keyring_module_signature=NULL;
KERNEL_PUBLIC keyring_t* KERNEL_INIT_WRITE keyring_user_signature=NULL;
KERNEL_PUBLIC notification_dispatcher_t* KERNEL_INIT_WRITE keyring_notification_dispatcher=NULL;



static void _keyring_handle_destructor(handle_t* handle){
	keyring_t* keyring=KERNEL_CONTAINEROF(handle,keyring_t,handle);
	ERROR("Delete keyring '%s'",keyring->name->data);
}



static void _dispatch_update_notification(keyring_t* keyring){
	keyring_update_notification_data_t data={
		keyring->handle.rb_node.key
	};
	notification_dispatcher_dispatch(keyring_notification_dispatcher,KEYRING_UPDATE_NOTIFICATION,&data,sizeof(keyring_update_notification_data_t));
}



KERNEL_INIT(){
	LOG("Initializing keyrings...");
	_keyring_allocator=omm_init("kernel.keyring",sizeof(keyring_t),8,2);
	rwlock_init(&(_keyring_allocator->lock));
	_keyring_key_allocator=omm_init("kernel.keyring.key",sizeof(keyring_key_t),8,2);
	rwlock_init(&(_keyring_key_allocator->lock));
	keyring_handle_type=handle_alloc("kernel.keyring",_keyring_handle_destructor);
	keyring_notification_dispatcher=notification_dispatcher_create();
	rwlock_init(&_keyring_creation_lock);
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
	rwlock_acquire_write(&_keyring_creation_lock);
	HANDLE_FOREACH(keyring_handle_type){
		keyring_t* keyring=KERNEL_CONTAINEROF(handle,keyring_t,handle);
		if (smm_equal(keyring->name,name_string)){
			rwlock_release_write(&_keyring_creation_lock);
			smm_dealloc(name_string);
			handle_release(handle);
			return keyring;
		}
	}
	keyring_t* out=omm_alloc(_keyring_allocator);
	out->name=name_string;
	handle_new(keyring_handle_type,&(out->handle));
	out->handle.acl=acl_create();
	rwlock_init(&(out->lock));
	out->head=NULL;
	rwlock_release_write(&_keyring_creation_lock);
	_dispatch_update_notification(out);
	return out;
}



KERNEL_PUBLIC void keyring_delete(keyring_t* keyring);



KERNEL_PUBLIC keyring_key_t* keyring_search(keyring_t* keyring,const char* name,u32 flags){
	SMM_TEMPORARY_STRING name_string=smm_alloc(name,0);
	rwlock_acquire_read(&(keyring->lock));
	keyring_key_t* key=keyring->head;
	for (;key&&!smm_equal(key->name,name_string);key=key->next);
	rwlock_release_read(&(keyring->lock));
	return key;
}



KERNEL_PUBLIC keyring_key_t* keyring_key_create(keyring_t* keyring,const char* name){
	string_t* name_string=smm_alloc(name,0);
	rwlock_acquire_write(&(keyring->lock));
	for (keyring_key_t* key=keyring->head;key;key=key->next){
		if (smm_equal(key->name,name_string)){
			rwlock_release_write(&(keyring->lock));
			smm_dealloc(name_string);
			return NULL;
		}
	}
	keyring_key_t* out=omm_alloc(_keyring_key_allocator);
	out->keyring=keyring;
	out->name=name_string;
	out->type=KEYRING_KEY_TYPE_NONE;
	out->flags=0;
	rwlock_init(&(out->lock));
	out->prev=NULL;
	out->next=keyring->head;
	if (keyring->head){
		keyring->head->prev=out;
	}
	else{
		keyring->head=out;
	}
	rwlock_release_write(&(keyring->lock));
	_dispatch_update_notification(keyring);
	return out;
}



KERNEL_PUBLIC void keyring_key_delete(keyring_key_t* key){
	ERROR("keyring_key_delete");
}



KERNEL_PUBLIC void keyring_key_update(keyring_key_t* key){
	_dispatch_update_notification(key->keyring);
}



KERNEL_PUBLIC bool keyring_key_process_rsa(keyring_key_t* key,rsa_number_t* in,rsa_number_t* out){
	rwlock_acquire_write(&(key->lock));
	bool ret=0;
	if (key->type==KEYRING_KEY_TYPE_RSA){
		rsa_state_process(&(key->data.rsa.state),in,RSA_PUBLIC_KEY,out);
		ret=1;
	}
	rwlock_release_write(&(key->lock));
	return ret;
}
