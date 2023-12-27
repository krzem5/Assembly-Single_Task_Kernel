#include <opengl/syscalls.h>
#include <sys/io.h>
#include <sys/syscall.h>
#include <sys/types.h>



SYS_PUBLIC void opengl_init(void){
	if (!opengl_syscalls_init()){
		printf("OpenGL module is not loaded\n");
		return;
	}
	printf("OpenGL module loaded\n");
}
