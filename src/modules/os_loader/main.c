#include <kernel/fs/fs.h>
#include <kernel/handle/handle.h>
#include <kernel/kernel.h>
#include <kernel/log/log.h>
#include <kernel/module/module.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#include <kernel/vfs/node.h>
#include <kernel/vfs/vfs.h>
#define KERNEL_LOG_NAME "os_loader"



#define EARLY_MODULE_ORDER_FILE "/boot/module/order.txt"



static _Bool _init(module_t* module){
	LOG("Loading early modules...");
	vfs_node_t* early_modile_order_file=vfs_lookup(NULL,EARLY_MODULE_ORDER_FILE);
	if (!early_modile_order_file){
		panic("Unable to locate early module order file");
	}
	char buffer[4096];
	u64 size=vfs_node_read(early_modile_order_file,0,buffer,4096);
	for (u64 i=0;i<size;){
		for (;i<size&&(!buffer[i]||buffer[i]==' '||buffer[i]=='\t'||buffer[i]=='\r'||buffer[i]=='\n');i++);
		u64 j=i;
		for (;j<size&&buffer[j]!='\n';j++);
		u64 k=j;
		for (;k&&(!buffer[k-1]||buffer[k-1]==' '||buffer[k-1]=='\t'||buffer[k-1]=='\r'||buffer[k-1]=='\n');k--);
		buffer[k]=0;
		if (buffer[i]&&buffer[i]!='#'){
			module_load(buffer+i);
		}
		i=j;
	}
	LOG("Searching for boot filesystem...");
	filesystem_t* boot_fs=NULL;
	HANDLE_FOREACH(HANDLE_TYPE_FS){
		handle_acquire(handle);
		filesystem_t* fs=handle->object;
		for (u8 i=0;i<16;i++){
			if (fs->uuid[i]!=kernel_data.boot_fs_uuid[i]){
				goto _check_next_fs;
			}
		}
		boot_fs=fs;
		break;
_check_next_fs:
		handle_release(handle);
	}
	if (!boot_fs){
		panic("Unable to finde boot filesystem");
	}
	vfs_mount(boot_fs,NULL);
	return 1;
}



static void _deinit(module_t* module){
	return;
}



MODULE_DECLARE(
	"os_loader",
	_init,
	_deinit
);
