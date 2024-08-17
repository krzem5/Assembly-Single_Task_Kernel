#include <sys/error/error.h>
#include <sys/fs/fs.h>
#include <sys/io/io.h>
#include <sys/string/string.h>
#include <sys/types.h>
#include <sys/util/options.h>



int main(int argc,const char** argv){
	if (!sys_options_parse_NEW(argc,argv,"")){
		return 1;
	}
	for (sys_fs_t fs=sys_fs_iter_start();fs;fs=sys_fs_iter_next(fs)){
		sys_fs_data_t data;
		if (SYS_IS_ERROR(sys_fs_get_data(fs,&data))){
			continue;
		}
		sys_io_print("%g: %s\t%s\n",data.guid,data.type,(data.mount_path[0]?data.mount_path:"<not mounted>"));
	}
	return 0;
}
