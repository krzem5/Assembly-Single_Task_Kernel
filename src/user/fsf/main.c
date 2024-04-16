#include <sys/error/error.h>
#include <sys/fs/fs.h>
#include <sys/io/io.h>
#include <sys/partition/partition.h>
#include <sys/string/string.h>
#include <sys/types.h>



int main(int argc,const char** argv){
	sys_io_print("Partitions:\n");
	for (sys_partition_t partition=sys_partition_iter_start();partition;partition=sys_partition_iter_next(partition)){
		sys_partition_data_t data;
		if (SYS_IS_ERROR(sys_partition_get_data(partition,&data))){
			continue;
		}
		sys_fs_data_t fs_data;
		sys_io_print("?d?p%u\t%s\t(%s)\t%p - %p\t%s\n",data.index,data.name,data.type,data.start_lba,data.end_lba,(SYS_IS_ERROR(sys_fs_get_data(data.fs,&fs_data))?"":fs_data.type));
	}
	sys_io_print("Filesystems:\n");
	for (sys_fs_descriptor_t fs_descriptor=sys_fs_descriptor_iter_start();fs_descriptor;fs_descriptor=sys_fs_descriptor_iter_next(fs_descriptor)){
		sys_fs_descriptor_data_t data;
		if (SYS_IS_ERROR(sys_fs_descriptor_get_data(fs_descriptor,&data))){
			continue;
		}
		sys_io_print("%s%s\n",data.name,((data.flags&SYS_FS_DESCRIPTOR_FLAG_CAN_FORMAT)?" (can format)":""));
	}
	return 0;
}
