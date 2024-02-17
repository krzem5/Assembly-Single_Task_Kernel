#include <kernel/elf/elf.h>
#include <kernel/error/error.h>
#include <kernel/handle/handle.h>
#include <kernel/log/log.h>
#include <kernel/mp/event.h>
#include <kernel/mp/process.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#include <kernel/vfs/node.h>
#include <kernel/vfs/vfs.h>
#include <test/test.h>
#define KERNEL_LOG_NAME "test_elf"



static void _test_arguments(void){
	error_t ret=elf_load("/bin/test_elf_send_results",0,NULL,0,NULL,0);
	TEST_ASSERT(!IS_ERROR(ret));
	if (IS_ERROR(ret)){
		return;
	}
	handle_t* handle=handle_lookup_and_acquire(ret,process_handle_type);
	process_t* process=handle->object;
	event_t* delete_event=process->event;
	handle_release(handle);
	event_await(delete_event,0);
}



void test_elf(void){
	LOG("Executing ELF tests...");
	SMM_TEMPORARY_STRING temp_name=smm_alloc("no-permission-node",0);
	vfs_node_t* no_permission_node=vfs_node_create_virtual(vfs_lookup(NULL,"/",0,0,0),NULL,temp_name);
	TEST_ASSERT(elf_load("/invalid/path",0,NULL,0,NULL,0)==ERROR_NOT_FOUND);
	no_permission_node->flags&=~VFS_NODE_PERMISSION_MASK;
	TEST_ASSERT(elf_load("/no-permission-node",0,NULL,0,NULL,0)==ERROR_NOT_FOUND);
	no_permission_node->flags=(no_permission_node->flags&(~VFS_NODE_PERMISSION_MASK))|(0444<<VFS_NODE_PERMISSION_SHIFT);
	TEST_ASSERT(elf_load("/no-permission-node",0,NULL,0,NULL,0)==ERROR_NOT_FOUND);
	no_permission_node->flags=(no_permission_node->flags&(~VFS_NODE_PERMISSION_MASK))|(0111<<VFS_NODE_PERMISSION_SHIFT);
	TEST_ASSERT(elf_load("/no-permission-node",0,NULL,0,NULL,0)==ERROR_NOT_FOUND);
	TEST_ASSERT(elf_load("/share/test/elf/invalid_header_signature",0,NULL,0,NULL,0)==ERROR_INVALID_FORMAT);
	TEST_ASSERT(elf_load("/share/test/elf/invalid_header_word_size",0,NULL,0,NULL,0)==ERROR_INVALID_FORMAT);
	TEST_ASSERT(elf_load("/share/test/elf/invalid_header_endianess",0,NULL,0,NULL,0)==ERROR_INVALID_FORMAT);
	TEST_ASSERT(elf_load("/share/test/elf/invalid_header_header_version",0,NULL,0,NULL,0)==ERROR_INVALID_FORMAT);
	TEST_ASSERT(elf_load("/share/test/elf/invalid_header_abi",0,NULL,0,NULL,0)==ERROR_INVALID_FORMAT);
	TEST_ASSERT(elf_load("/share/test/elf/invalid_header_type",0,NULL,0,NULL,0)==ERROR_INVALID_FORMAT);
	TEST_ASSERT(elf_load("/share/test/elf/invalid_header_machine",0,NULL,0,NULL,0)==ERROR_INVALID_FORMAT);
	TEST_ASSERT(elf_load("/share/test/elf/invalid_header_version",0,NULL,0,NULL,0)==ERROR_INVALID_FORMAT);
	TEST_ASSERT(elf_load("/share/test/elf/multiple_interpreters",0,NULL,0,NULL,0)==ERROR_INVALID_FORMAT);
	TEST_ASSERT(elf_load("/share/test/elf/unterminated_interpreter",0,NULL,0,NULL,0)==ERROR_INVALID_FORMAT);
	TEST_ASSERT(elf_load("/share/test/elf/interpreter_invalid_path",0,NULL,0,NULL,0)==ERROR_NOT_FOUND);
	no_permission_node->flags&=~VFS_NODE_PERMISSION_MASK;
	TEST_ASSERT(elf_load("/share/test/elf/interpreter_no_permissions",0,NULL,0,NULL,0)==ERROR_NOT_FOUND);
	no_permission_node->flags=(no_permission_node->flags&(~VFS_NODE_PERMISSION_MASK))|(0444<<VFS_NODE_PERMISSION_SHIFT);
	TEST_ASSERT(elf_load("/share/test/elf/interpreter_no_permissions",0,NULL,0,NULL,0)==ERROR_NOT_FOUND);
	no_permission_node->flags=(no_permission_node->flags&(~VFS_NODE_PERMISSION_MASK))|(0111<<VFS_NODE_PERMISSION_SHIFT);
	TEST_ASSERT(elf_load("/share/test/elf/interpreter_no_permissions",0,NULL,0,NULL,0)==ERROR_NOT_FOUND);
	TEST_ASSERT(elf_load("/share/test/elf/interpreter_invalid_header_signature",0,NULL,0,NULL,0)==ERROR_INVALID_FORMAT);
	TEST_ASSERT(elf_load("/share/test/elf/interpreter_invalid_header_word_size",0,NULL,0,NULL,0)==ERROR_INVALID_FORMAT);
	TEST_ASSERT(elf_load("/share/test/elf/interpreter_invalid_header_endianess",0,NULL,0,NULL,0)==ERROR_INVALID_FORMAT);
	TEST_ASSERT(elf_load("/share/test/elf/interpreter_invalid_header_header_version",0,NULL,0,NULL,0)==ERROR_INVALID_FORMAT);
	TEST_ASSERT(elf_load("/share/test/elf/interpreter_invalid_header_abi",0,NULL,0,NULL,0)==ERROR_INVALID_FORMAT);
	TEST_ASSERT(elf_load("/share/test/elf/interpreter_invalid_header_type",0,NULL,0,NULL,0)==ERROR_INVALID_FORMAT);
	TEST_ASSERT(elf_load("/share/test/elf/interpreter_invalid_header_machine",0,NULL,0,NULL,0)==ERROR_INVALID_FORMAT);
	TEST_ASSERT(elf_load("/share/test/elf/interpreter_invalid_header_version",0,NULL,0,NULL,0)==ERROR_INVALID_FORMAT);
	vfs_node_dettach_external_child(no_permission_node);
	vfs_node_delete(no_permission_node);
	// ################################ Positive tests ################################
	_test_arguments();
	// argc
	// argv[...]
	// environ[...]
	// auxv: phent
	// auxv: pagesz
	// auxv: base
	// auxv: flags
	// auxv: entry
	// auxv: platform
	// auxv: hwcap
	// auxv: hwcap2
	// auxv: execfn
}
