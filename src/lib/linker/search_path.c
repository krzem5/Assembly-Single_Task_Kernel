#include <sys/error/error.h>
#include <sys/fd/fd.h>
#include <sys/types.h>



static const char* _library_search_directories[]={
	"/lib",
	".",
	NULL
};



u64 search_path_find_library(const char* name,char* buffer,u32 buffer_length){
	for (u32 i=0;_library_search_directories[i];i++){
		sys_fd_t fd=sys_fd_open(0,_library_search_directories[i],0);
		if (SYS_IS_ERROR(fd)){
			continue;
		}
		sys_fd_t child=sys_fd_open(fd,name,SYS_FD_FLAG_READ);
		sys_fd_close(fd);
		if (!SYS_IS_ERROR(child)){
			sys_fd_path(child,buffer,buffer_length);
			return child;
		}
	}
	return 0;
}
