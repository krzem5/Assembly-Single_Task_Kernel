#include <kernel/fs/fs.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#include <kernel/vfs2/name.h>
#include <kernel/vfs2/node.h>



vfs2_node_t* vfs2_node_create(struct _FILESYSTEM2* fs,const char* name,u32 name_length){
	vfs2_node_t* out=fs->functions->create();
	if (!out){
		return NULL;
	}
	out->flags=0;
	lock_init(&(out->lock));
	out->name=vfs2_name_alloc(name,name_length);
	out->relatives.parent=NULL;
	out->relatives.prev_sibling=NULL;
	out->relatives.next_sibling=NULL;
	out->relatives.child=NULL;
	out->fs=fs;
	out->functions=fs->functions;
	return out;
}
