#include <sys/fd.h>
#include <sys/types.h>



static const char* _library_search_directories[]={
	"/lib",
	".",
	NULL
};



u64 search_path_find_library(const char* name,char* buffer,u32 buffer_length){
	for (u32 i=0;_library_search_directories[i];i++){
		s64 fd=sys_fd_open(0,_library_search_directories[i],0);
		if (fd<=0){
			continue;
		}
		s64 child=sys_fd_open(fd,name,SYS_FD_FLAG_READ);
		sys_fd_close(fd);
		if (child>0){
			sys_fd_path(child,buffer,buffer_length);
			return child;
		}
	}
	return 0;
}
