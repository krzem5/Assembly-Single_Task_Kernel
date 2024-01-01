#include <sys2/error/error.h>
#include <sys2/fd/fd.h>
#include <sys2/types.h>



static const char* _library_search_directories[]={
	"/lib",
	".",
	NULL
};



u64 search_path_find_library(const char* name,char* buffer,u32 buffer_length){
	for (u32 i=0;_library_search_directories[i];i++){
		sys2_fd_t fd=sys2_fd_open(0,_library_search_directories[i],0);
		if (SYS2_IS_ERROR(fd)){
			continue;
		}
		sys2_fd_t child=sys2_fd_open(fd,name,SYS2_FD_FLAG_READ);
		sys2_fd_close(fd);
		if (!SYS2_IS_ERROR(child)){
			sys2_fd_path(child,buffer,buffer_length);
			return child;
		}
	}
	return 0;
}
