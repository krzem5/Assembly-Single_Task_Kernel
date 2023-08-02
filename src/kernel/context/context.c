#include <kernel/fs/fs.h>
#include <kernel/log/log.h>
#include <kernel/types.h>
#define KERNEL_LOG_NAME "context"



typedef struct _CONTEXT_CPU{
	u64 rip;
	u64 rflags;
	u64 fs;
	u64 gs;
	u64 rax;
	u64 rbx;
	u64 rdx;
	u64 rsi;
	u64 rdi;
	u64 rsp;
	u64 rbp;
	u64 r8;
	u64 r9;
	u64 r10;
	u64 r12;
	u64 r13;
	u64 r14;
	u64 r15;
	u64 ymm0[4];
	u64 ymm1[4];
	u64 ymm2[4];
	u64 ymm3[4];
	u64 ymm4[4];
	u64 ymm5[4];
	u64 ymm6[4];
	u64 ymm7[4];
	u64 ymm8[4];
	u64 ymm9[4];
	u64 ymm10[4];
	u64 ymm11[4];
	u64 ymm12[4];
	u64 ymm13[4];
	u64 ymm14[4];
	u64 ymm15[4];
} context_cpu_t;



typedef struct _CONTEXT_FD{
	u16 index;
	u16 path_length;
	u8 flags;
	u64 offset;
	char path[512];
} context_fd_t;



typedef struct _CONTEXT_MMAP_REGION{
	u64 virtual_address;
	u64 physical_address;
	u64 length;
	u64 flags;
} context_mmap_region_t;



typedef struct _CONTEXT_RAM_REGION{
	u64 physical_address;
	u64 length;
	u64 compressed_length;
} context_ram_region_t;



typedef	struct _CONTEXT{
	u16 cpu_count;
	u16 fd_count;
	u64 mmap_region_count;
	u64 ram_region_count;
} context_t;



static const char* _context_file_path=":/__user_context";



static void _get_context_file_path(char* path){
	const fs_file_system_t* fs;
	for (u8 i=0;1;i++){
		fs=fs_get_file_system(i);
		if (!fs){
			path[0]=0;
			return;
		}
		if (fs->flags&FS_FILE_SYSTEM_FLAG_BOOT){
			break;
		}
	}
	u8 i=0;
	for (;fs->name[i];i++){
		path[i]=fs->name[i];
	}
	u8 j=0;
	for (;_context_file_path[j];j++){
		path[i+j]=_context_file_path[j];
	}
	path[i+j]=0;
}



void context_load(void){
	LOG("Loading user context...");
	char path[64];
	_get_context_file_path(path);
	fs_node_t* node=fs_get_node(0,path,0);
	if (!node){
		LOG("Context file not found");
		return;
	}
	LOG("Found context file '%s'",path);
	fs_delete_node(node);
	for (;;);
}



void context_save(void){
	LOG("Saving user context...");
	char path[64];
	_get_context_file_path(path);
	fs_node_t* node=fs_get_node(0,path,FS_NODE_TYPE_FILE);
	if (!node){
		ERROR("Unable to open open context file '%s'",path);
		return;
	}
	for (;;);
}
