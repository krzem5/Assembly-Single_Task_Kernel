#include <kernel/elf/elf.h>
#include <kernel/log/log.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#include <kernel/vfs/node.h>
#include <kernel/vfs/vfs.h>
#include <test/test.h>
#define KERNEL_LOG_NAME "test_elf"



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
	vfs_node_dettach_external_child(no_permission_node);
	vfs_node_delete(no_permission_node);
	// ################################ Negative tests ################################
	// invalid interpreter path ==> ERROR_NOT_FOUND
	// invalid interpreter path permissions ==> ERROR_NOT_FOUND
	// invalid interpreter header: e_ident.signature ==> ERROR_INVALID_FORMAT
	// invalid interpreter header: e_ident.word_size ==> ERROR_INVALID_FORMAT
	// invalid interpreter header: e_ident.endianess ==> ERROR_INVALID_FORMAT
	// invalid interpreter header: e_ident.header_version ==> ERROR_INVALID_FORMAT
	// invalid interpreter header: e_ident.abi ==> ERROR_INVALID_FORMAT
	// invalid interpreter header: e_type ==> ERROR_INVALID_FORMAT
	// invalid interpreter header: machine ==> ERROR_INVALID_FORMAT
	// invalid interpreter header: version ==> ERROR_INVALID_FORMAT
	// ################################ Positive tests ################################
	// argc
	// argv[...]
	// environ[...]
	// auxv: phdr
	// auxv: phent
	// auxv: phnum
	// auxv: pagesz
	// auxv: base
	// auxv: flags
	// auxv: entry
	// auxv: platform
	// auxv: hwcap
	// auxv: hwcap2
	// auxv: execfn
}
