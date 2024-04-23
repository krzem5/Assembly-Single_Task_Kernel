#include <linker/alloc.h>
#include <sys/error/error.h>
#include <sys/fd/fd.h>
#include <sys/memory/memory.h>
#include <sys/string/string.h>
#include <sys/types.h>



static const char* _library_default_search_directories="/lib:/:.";
static bool _library_search_directories_was_allocated=0;
static char* _library_search_directories=NULL;



u64 search_path_find_library(const char* name,char* buffer,u32 buffer_length){
	if (!_library_search_directories){
		_library_search_directories_was_allocated=0;
		_library_search_directories=(char*)_library_default_search_directories;
	}
	for (u32 i=0;_library_search_directories[i];){
		u32 length=0;
		for (;_library_search_directories[i+length]&&_library_search_directories[i+length]!=':';length++);
		if (length>=buffer_length){
			return SYS_ERROR_NO_SPACE;
		}
		sys_memory_copy(_library_search_directories+i,buffer,length);
		buffer[length]=0;
		i+=length+(_library_search_directories[i+length]==':');
		sys_fd_t fd=sys_fd_open(0,buffer,0);
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
	return SYS_ERROR_NOT_FOUND;
}



const char* search_path_update_search_directories(const char* new){
	if (!new){
		return _library_search_directories;
	}
	if (_library_search_directories_was_allocated){
		dealloc(_library_search_directories);
	}
	u32 size=sys_string_length(new)+1;
	_library_search_directories_was_allocated=1;
	_library_search_directories=alloc(size);
	sys_memory_copy(new,_library_search_directories,size);
	return _library_search_directories;
}
