#include <sys/error/error.h>
#include <sys/fs/fs.h>
#include <sys/io/io.h>
#include <sys/string/string.h>
#include <sys/types.h>



int main(int argc,const char** argv){
	// for (sys_fs_t fs=sys_fs_iter_start();fs;fs=sys_fs_iter_next(fs)){
	// 	sys_fs_data_t data;
	// 	if (SYS_IS_ERROR(sys_fs_get_data(fs,&data))){
	// 		continue;
	// 	}
	// 	sys_io_print("%g: %s\t%s\n",data.guid,data.type,(data.mount_path[0]?data.mount_path:"<not mounted>"));
	// }
	for (sys_fs_descriptor_t fs_descriptor=sys_fs_descriptor_iter_start();fs_descriptor;fs_descriptor=sys_fs_descriptor_iter_next(fs_descriptor)){
		sys_fs_descriptor_data_t data;
		if (SYS_IS_ERROR(sys_fs_descriptor_get_data(fs_descriptor,&data))){
			continue;
		}
		sys_io_print("%s%s\n",data.name,((data.flags&SYS_FS_DESCRIPTOR_FLAG_CAN_FORMAT)?" (can format)":""));
	}
	return 0;
}
