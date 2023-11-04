#include <kernel/elf/elf.h>
#include <kernel/fs/fs.h>
#include <kernel/handle/handle.h>
#include <kernel/kernel.h>
#include <kernel/log/log.h>
#include <kernel/memory/mmap.h>
#include <kernel/module/module.h>
#include <kernel/mp/process.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#include <kernel/vfs/vfs.h>
#define KERNEL_LOG_NAME "os_loader"



#define EARLY_MODULE_ORDER_FILE "/boot/module/order.txt"
#define LATE_MODULE_ORDER_FILE "/boot/module/order.txt" // Same file name, different filesystem



static void _load_modules_from_order_file(const char* order_file_path){
	vfs_node_t* file=vfs_lookup(NULL,order_file_path);
	if (!file){
		panic("Unable to locate module order file");
	}
	mmap_region_t* region=mmap_alloc(&(process_kernel->mmap),0,0,NULL,MMAP_REGION_FLAG_NO_FILE_WRITEBACK|MMAP_REGION_FLAG_VMM_NOEXECUTE|MMAP_REGION_FLAG_VMM_READWRITE,file);
	char* buffer=(char*)(region->rb_node.key);
	for (u64 i=0;i<region->length&&buffer[i];){
		for (;i<region->length&&(buffer[i]==' '||buffer[i]=='\t'||buffer[i]=='\r'||buffer[i]=='\n');i++);
		if (!buffer[i]){
			break;
		}
		u64 j=i;
		for (;j<region->length&&buffer[j]!='\n';j++);
		u64 k=j;
		for (;k&&(buffer[k-1]==' '||buffer[k-1]=='\t'||buffer[k-1]=='\r'||buffer[k-1]=='\n');k--);
		if (buffer[i]&&buffer[i]!='#'){
			SMM_TEMPORARY_STRING module_name=smm_alloc(buffer+i,k-i);
			module_load(module_name->data);
		}
		i=j;
	}
	mmap_dealloc(&(process_kernel->mmap),region->rb_node.key,region->length);
}



static _Bool _init(module_t* module){
	LOG("Loading early modules...");
	_load_modules_from_order_file(EARLY_MODULE_ORDER_FILE);
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
		panic("Unable to find boot filesystem");
	}
	vfs_mount(boot_fs,NULL);
	LOG("Loading late modules...");
	_load_modules_from_order_file(LATE_MODULE_ORDER_FILE);
#if KERNEL_COVERAGE_ENABLED
	module_load("coverage");
#endif
	LOG("Loading user shell...");
	if (!elf_load(vfs_lookup(NULL,"/shell.elf"))){
		panic("Unable to load user shell");
	}
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
