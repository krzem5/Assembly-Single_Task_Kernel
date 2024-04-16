#include <sys/drive/drive.h>
#include <sys/error/error.h>
#include <sys/fs/fs.h>
#include <sys/io/io.h>
#include <sys/partition/partition.h>
#include <sys/string/string.h>
#include <sys/types.h>



int main(int argc,const char** argv){
	sys_io_print("Unformatted drives:\n");
	for (sys_drive_t drive=sys_drive_iter_start();drive;drive=sys_drive_iter_next(drive)){
		sys_drive_data_t drive_data;
		if (SYS_IS_ERROR(sys_drive_get_data(drive,&drive_data))||drive_data.partition_table_type[0]){
			continue;
		}
		sys_io_print("\x1b[1m%s%ud%u\x1b[0m\t%p x %u\n",drive_data.type,drive_data.controller_index,drive_data.device_index,drive_data.block_count,drive_data.block_size);
	}
	sys_io_print("Partitions:\n");
	for (sys_partition_t partition=sys_partition_iter_start();partition;partition=sys_partition_iter_next(partition)){
		sys_partition_data_t partition_data;
		sys_drive_data_t drive_data;
		if (SYS_IS_ERROR(sys_partition_get_data(partition,&partition_data))||SYS_IS_ERROR(sys_drive_get_data(partition_data.drive,&drive_data))){
			continue;
		}
		sys_fs_data_t fs_data;
		sys_io_print("\x1b[1m%s%ud%up%u\x1b[0m\t%s\t(%s)\t%p - %p\t%s\n",drive_data.type,drive_data.controller_index,drive_data.device_index,partition_data.index,partition_data.name,partition_data.type,partition_data.start_lba,partition_data.end_lba,(SYS_IS_ERROR(sys_fs_get_data(partition_data.fs,&fs_data))?"":fs_data.type));
	}
	sys_io_print("Filesystems:\n");
	for (sys_fs_descriptor_t fs_descriptor=sys_fs_descriptor_iter_start();fs_descriptor;fs_descriptor=sys_fs_descriptor_iter_next(fs_descriptor)){
		sys_fs_descriptor_data_t data;
		if (SYS_IS_ERROR(sys_fs_descriptor_get_data(fs_descriptor,&data))){
			continue;
		}
		sys_io_print("\x1b[1m%s\x1b[0m\t%s\n",data.name,((data.flags&SYS_FS_DESCRIPTOR_FLAG_CAN_FORMAT)?"(can format)":""));
	}
	return 0;
}
