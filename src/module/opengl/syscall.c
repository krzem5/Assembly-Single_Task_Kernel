#include <kernel/log/log.h>
#include <kernel/syscall/syscall.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#include <opengl/opengl.h>
#define KERNEL_LOG_NAME "opengl_syscalls"



static const syscall_callback_t _opengl_syscall_functions[]={
	NULL
};



void opengl_syscall_init(void){
	LOG("Initializing OpenGL syscalls...");
	syscall_create_table("opengl",_opengl_syscall_functions,sizeof(_opengl_syscall_functions)/sizeof(const syscall_callback_t));
}
