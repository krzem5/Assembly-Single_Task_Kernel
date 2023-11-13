#include <core/fd.h>
#include <core/types.h>



u64 resolve_library_name(const char* name,char* buffer,u32 buffer_length){
	const char* _library_search_directories[]={
		".",
		"/lib",
		NULL
	};
	for (u32 i=0;_library_search_directories[i];i++){
		s64 fd=fd_open(0,_library_search_directories[i],0);
		if (fd<=0){
			continue;
		}
		s64 child=fd_open(fd,name,FD_FLAG_READ);
		fd_close(fd);
		if (child>0){
			fd_path(child,buffer,buffer_length);
			return child;
		}
	}
	return 0;
}
