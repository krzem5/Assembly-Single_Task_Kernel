#include <kernel/acl/acl.h>
#include <kernel/handle/handle.h>
#include <kernel/handle/handle_list.h>
#include <kernel/lock/rwlock.h>
#include <kernel/log/log.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/module/module.h>
#include <kernel/mp/thread.h>
#include <kernel/types.h>
#include <kernel/util/string.h>
#include <kernel/util/util.h>
#include <opengl/opengl.h>
#define KERNEL_LOG_NAME "opengl"



static omm_allocator_t* KERNEL_INIT_WRITE _opengl_driver_instance_allocator=NULL;
static omm_allocator_t* KERNEL_INIT_WRITE _opengl_state_allocator=NULL;

handle_type_t KERNEL_INIT_WRITE opengl_driver_instance_handle_type=0;
handle_type_t KERNEL_INIT_WRITE opengl_state_handle_type=0;



MODULE_INIT(){
	LOG("Initializing OpenGL...");
	_opengl_driver_instance_allocator=omm_init("opengl.driver_instance",sizeof(opengl_driver_instance_t),8,2);
	rwlock_init(&(_opengl_driver_instance_allocator->lock));
	_opengl_state_allocator=omm_init("opengl.state",sizeof(opengl_state_t),8,2);
	rwlock_init(&(_opengl_state_allocator->lock));
	opengl_driver_instance_handle_type=handle_alloc("opengl.driver_instance",NULL);
	opengl_state_handle_type=handle_alloc("opengl.state",NULL);
}



KERNEL_PUBLIC opengl_driver_instance_t* opengl_create_driver_instance(const opengl_driver_t* driver,const char* renderer,void* ctx){
	LOG("Creating new OpenGL driver instance '%s/%s'...",driver->name,renderer);
	opengl_driver_instance_t* out=omm_alloc(_opengl_driver_instance_allocator);
	handle_new(opengl_driver_instance_handle_type,&(out->handle));
	str_copy(renderer,out->renderer,64);
	out->driver=driver;
	out->ctx=ctx;
	return out;
}



opengl_state_t* opengl_create_state(opengl_driver_instance_t* driver_instance){
	opengl_state_t* out=omm_alloc(_opengl_state_allocator);
	handle_new(opengl_state_handle_type,&(out->handle));
	handle_list_push(&(THREAD_DATA->process->handle_list),&(out->handle));
	out->handle.acl=acl_create();
	acl_set(out->handle.acl,THREAD_DATA->process,0,OPENGL_STATE_ACL_FLAG_SEND_COMMANDS);
	out->driver_instance=driver_instance;
	out->ctx=NULL;
	out->framebuffer=NULL;
	bool ret=driver_instance->driver->init_state(driver_instance,out);
	if (ret){
		return out;
	}
	handle_destroy(&(out->handle));
	omm_dealloc(_opengl_state_allocator,out);
	return NULL;
}



void opengl_delete_state(opengl_state_t* state){
	panic("opengl_delete_state");
}
