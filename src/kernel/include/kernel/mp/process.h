#ifndef _KERNEL_MP_PROCESS_H_
#define _KERNEL_MP_PROCESS_H_ 1
#include <kernel/handle/handle.h>
#include <kernel/mp/_mp_types.h>



#define PROCESS_ACL_FLAG_CREATE_THREAD 1
#define PROCESS_ACL_FLAG_TERMINATE 2
#define PROCESS_ACL_FLAG_SWITCH_USER 4



typedef struct _PROCESS_QUERY_USER_DATA{
	u64 pid;
	u64 ppid;
	char name[256];
	char image[4096];
	u32 uid;
	u32 gid;
	char vfs_cwd[4096];
	u64 fd_stdin;
	u64 fd_stdout;
	u64 fd_stderr;
} process_query_user_data_t;



extern handle_type_t process_handle_type;
extern process_t* process_kernel;



void process_init(void);



process_t* process_create(const char* image,const char* name,u64 mmap_bottom_address,u64 mmap_top_address);



bool process_is_root(void);



id_flags_t process_get_id_flags(void);



#endif
