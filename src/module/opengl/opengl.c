#include <kernel/handle/handle.h>
#include <kernel/lock/spinlock.h>
#include <kernel/log/log.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#include <opengl/opengl.h>
#define KERNEL_LOG_NAME "opengl"



static omm_allocator_t* _opengl_driver_instance_allocator=NULL;
static handle_type_t _opengl_driver_instance_handle_type=0;



void opengl_init(void){
	LOG("Initializing OpenGL...");
	_opengl_driver_instance_allocator=omm_init("opengl_driver_instance",sizeof(opengl_driver_instance_t),8,2,pmm_alloc_counter("omm_opengl_driver_instance"));
	spinlock_init(&(_opengl_driver_instance_allocator->lock));
	_opengl_driver_instance_handle_type=handle_alloc("opengl_driver_instance",NULL);
}



KERNEL_PUBLIC opengl_driver_instance_t* opengl_create_driver_instance(const opengl_driver_t* driver,const char* renderer,void* ctx){
	LOG("Creating new OpenGL driver instance '%s/%s'...",driver->name,renderer);
	opengl_driver_instance_t* out=omm_alloc(_opengl_driver_instance_allocator);
	handle_new(out,_opengl_driver_instance_handle_type,&(out->handle));
	strcpy(out->renderer,renderer,64);
	out->driver=driver;
	out->ctx=ctx;
	handle_finish_setup(&(out->handle));
	return out;
}
