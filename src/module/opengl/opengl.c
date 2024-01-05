#include <kernel/acl/acl.h>
#include <kernel/handle/handle.h>
#include <kernel/handle/handle_list.h>
#include <kernel/lock/spinlock.h>
#include <kernel/log/log.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/mp/thread.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#include <opengl/opengl.h>
#define KERNEL_LOG_NAME "opengl"



static omm_allocator_t* _opengl_driver_instance_allocator=NULL;
static omm_allocator_t* _opengl_state_allocator=NULL;

handle_type_t opengl_driver_instance_handle_type=0;
handle_type_t opengl_state_handle_type=0;



void opengl_init(void){
	LOG("Initializing OpenGL...");
	_opengl_driver_instance_allocator=omm_init("opengl_driver_instance",sizeof(opengl_driver_instance_t),8,2,pmm_alloc_counter("omm_opengl_driver_instance"));
	spinlock_init(&(_opengl_driver_instance_allocator->lock));
	_opengl_state_allocator=omm_init("opengl_state",sizeof(opengl_state_t),8,2,pmm_alloc_counter("omm_opengl_state"));
	spinlock_init(&(_opengl_state_allocator->lock));
	opengl_driver_instance_handle_type=handle_alloc("opengl_driver_instance",NULL);
	opengl_state_handle_type=handle_alloc("opengl_state",NULL);
}



KERNEL_PUBLIC opengl_driver_instance_t* opengl_create_driver_instance(const opengl_driver_t* driver,const char* renderer,void* ctx){
	LOG("Creating new OpenGL driver instance '%s/%s'...",driver->name,renderer);
	opengl_driver_instance_t* out=omm_alloc(_opengl_driver_instance_allocator);
	handle_new(out,opengl_driver_instance_handle_type,&(out->handle));
	strcpy(out->renderer,renderer,64);
	out->driver=driver;
	out->ctx=ctx;
	handle_finish_setup(&(out->handle));
	return out;
}



opengl_state_t* opengl_create_state(opengl_driver_instance_t* driver_instance){
	opengl_state_t* out=omm_alloc(_opengl_state_allocator);
	handle_new(out,opengl_state_handle_type,&(out->handle));
	handle_list_push(&(THREAD_DATA->process->handle_list),&(out->handle));
	out->handle.acl=acl_create();
	acl_set(out->handle.acl,THREAD_DATA->process,0,OPENGL_STATE_ACL_FLAG_SEND_COMMANDS);
	out->driver_instance=driver_instance;
	out->ctx=NULL;
	out->framebuffer=NULL;
	_Bool ret=driver_instance->driver->init_state(driver_instance,out);
	handle_finish_setup(&(out->handle));
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
