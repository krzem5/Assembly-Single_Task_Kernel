#include <kernel/handle/handle.h>
#include <kernel/kernel.h>
#include <kernel/lock/lock.h>
#include <kernel/log/log.h>
#include <kernel/memory/kmm.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "handle"



#define USE_RB_TREE 1
#if USE_RB_TREE
#define RB_ROOT_INIT RB_NIL_NODE
#else
#define RB_ROOT_INIT NULL
#endif

#define RB_NIL_NODE (&_handle_rb_nil_node)



static handle_t _handle_rb_nil_node={
	.rb_color=0,
	.rb_parent=NULL,
	.rb_left=NULL,
	.rb_right=NULL
};

handle_type_data_t* handle_type_data;
handle_type_t handle_type_count;



static void _handle_add_node_to_tree(handle_t** root,handle_t* node){
#if USE_RB_TREE
#define x node
	node->rb_left=RB_NIL_NODE;
	node->rb_right=RB_NIL_NODE;
	if (*root==RB_NIL_NODE){
		x->rb_parent=NULL;
		x->rb_color=0;
		*root=x;
		return;
	}
	x->rb_color=1;
	handle_t* y=*root;
	for (;y->rb_right!=RB_NIL_NODE;y=y->rb_right);
	x->rb_parent=y;
	y->rb_right=x;
	if (!y->rb_parent){
		return;
	}
	while (x->rb_parent&&x->rb_parent->rb_color){
		y=x->rb_parent;
		y->rb_color=0;
		y=y->rb_parent;
		y->rb_color=1;
		handle_t* z=y->rb_left;
		if (z->rb_color){
			z->rb_color=0;
			x=y;
			continue;
		}
		z=y->rb_right;
		y->rb_right=z->rb_left;
		z->rb_left->rb_parent=y;
		z->rb_parent=y->rb_parent;
		if (y->rb_parent){
			y->rb_parent->rb_right=z;
		}
		else{
			*root=z;
		}
		z->rb_left=y;
		y->rb_parent=z;
	}
	(*root)->rb_color=0;
#undef x
#else
	node->rb_parent=&_handle_rb_nil_node;
	node->rb_left=NULL;
	node->rb_right=*root;
	if (*root){
		(*root)->rb_left=node;
	}
	*root=node;
#endif
}



static handle_t* _handle_get_node_from_tree(handle_t* root,handle_id_t id){
#if USE_RB_TREE
	for (handle_t* x=root;x!=RB_NIL_NODE;x=x->rb_nodes[x->id<id]){
		if (x->id==id){
			return x;
		}
	}
	return NULL;
#else
	for (handle_t* handle=root;handle;handle=handle->rb_right){
		if (handle->id==id){
			return handle;
		}
	}
	return NULL;
#endif
}



static KERNEL_INLINE void _handle_replace_tree_node(handle_t** root,handle_t* old,handle_t* new){
	if (old->rb_parent){
		old->rb_parent->rb_nodes[old->rb_parent->rb_right==old]=new;
	}
	else{
		*root=new;
	}
}



static void _handle_remove_node_from_tree(handle_t** root,handle_t* node){
#if USE_RB_TREE
#define x node
	_Bool skip_recursive_fix=x->rb_color;
	handle_t* z;
	handle_t* z_parent;
	if (x->rb_left!=RB_NIL_NODE&&x->rb_right!=RB_NIL_NODE){
		handle_t* y=x->rb_right;
		for (;y->rb_left!=RB_NIL_NODE;y=y->rb_left);
		skip_recursive_fix=y->rb_color;
		z=y->rb_right;
		z_parent=y;
		if (y->rb_parent!=x){
			y->rb_parent->rb_left=y->rb_right;
			y->rb_right=x->rb_right;
			x->rb_right->rb_parent=y;
		}
		_handle_replace_tree_node(root,x,y);
		y->rb_parent=x->rb_parent;
		y->rb_left=x->rb_left;
		y->rb_left->rb_parent=y;
		y->rb_color=x->rb_color;
	}
	else{
		z=x->rb_nodes[x->rb_left==RB_NIL_NODE];
		_handle_replace_tree_node(root,x,z);
		z_parent=x->rb_parent;
		z->rb_parent=z_parent;
	}
	if (skip_recursive_fix){
		return;
	}
	while (z->rb_parent&&!z->rb_color){
		_Bool i=(z==z_parent->rb_left);
		_Bool j=i^1;
		x=z_parent->rb_nodes[i];
		if (x->rb_color){
			x->rb_color=0;
			z_parent->rb_color=1;
			handle_t* y=x->rb_nodes[j];
			z_parent->rb_nodes[i]=y;
			y->rb_parent=z_parent;
			x->rb_parent=z_parent->rb_parent;
			_handle_replace_tree_node(root,z_parent,x);
			x->rb_nodes[j]=z_parent;
			z_parent->rb_parent=x;
			x=y;
		}
		if (!x->rb_left->rb_color&&!x->rb_right->rb_color){
			x->rb_color=1;
			z=z_parent;
			z_parent=z->rb_parent;
			continue;
		}
		if (!x->rb_nodes[i]->rb_color){
			x->rb_nodes[j]->rb_color=0;
			x->rb_color=1;
			handle_t* y=x->rb_nodes[j];
			x->rb_nodes[j]=y->rb_nodes[i];
			y->rb_nodes[i]->rb_parent=x;
			y->rb_parent=x->rb_parent;
			x->rb_parent->rb_nodes[x==x->rb_parent->rb_right]=y;
			y->rb_nodes[i]=x;
			x->rb_parent=y;
			x=z_parent->rb_nodes[i];
		}
		x->rb_color=z_parent->rb_color;
		z_parent->rb_color=0;
		x->rb_nodes[i]->rb_color=0;
		z_parent->rb_nodes[i]=x->rb_nodes[j];
		x->rb_nodes[j]->rb_parent=z_parent;
		x->rb_parent=z_parent->rb_parent;
		_handle_replace_tree_node(root,z_parent,x);
		x->rb_nodes[j]=z_parent;
		z_parent->rb_parent=x;
		(*root)->rb_color=0;
		return;
	}
	z->rb_color=0;
#undef x
#else
	if (node->rb_left){
		node->rb_left->rb_right=node->rb_right;
	}
	else{
		*root=node->rb_right;
	}
	if (node->rb_right){
		node->rb_right->rb_left=node->rb_left;
	}
#endif
}



void handle_init(void){
	LOG("Initializing handle types...");
	handle_type_count=HANDLE_TYPE_ANY+1;
	for (const handle_descriptor_t*const* descriptor=(void*)kernel_section_handle_start();(u64)descriptor<kernel_section_handle_end();descriptor++){
		if (*descriptor){
			*((*descriptor)->var)=handle_type_count;
			handle_type_count++;
		}
	}
	INFO("Handle type count: %u",handle_type_count);
	handle_type_data=kmm_alloc(handle_type_count*sizeof(handle_type_data_t));
	memset(handle_type_data->name,0,HANDLE_NAME_LENGTH);
	memcpy(handle_type_data->name,"any",3);
	lock_init(&(handle_type_data->lock));
	handle_type_data->delete_callback=NULL;
	handle_type_data->rb_root=RB_ROOT_INIT;
	handle_type_data->count=0;
	handle_type_data->active_count=0;
	for (const handle_descriptor_t*const* descriptor=(void*)kernel_section_handle_start();(u64)descriptor<kernel_section_handle_end();descriptor++){
		if (*descriptor){
			handle_type_data_t* type_data=handle_type_data+(*((*descriptor)->var));
			memcpy_lowercase(type_data->name,(*descriptor)->name,HANDLE_NAME_LENGTH);
			lock_init(&(type_data->lock));
			type_data->delete_callback=(*descriptor)->delete_callback;
			type_data->rb_root=RB_ROOT_INIT;
			type_data->count=0;
			type_data->active_count=0;
		}
	}
}



void handle_new(void* object,handle_type_t type,handle_t* out){
	if (type==HANDLE_TYPE_ANY||type>=handle_type_count){
		panic("Invalid handle type");
	}
	out->rc=1;
	out->object_offset=((s64)object)-((s64)out);
	if (((u64)out)+out->object_offset!=((u64)object)){
		panic("Wrong object offset");
	}
	handle_type_data_t* type_data=handle_type_data+type;
	lock_init(&(out->lock));
	lock_acquire_exclusive(&(type_data->lock));
	out->id=HANDLE_ID_CREATE(type,type_data->active_count);
	handle_type_data->active_count++;
	type_data->count++;
	type_data->active_count++;
	_handle_add_node_to_tree(&(type_data->rb_root),out);
	lock_release_exclusive(&(type_data->lock));
}



handle_t* handle_lookup_and_acquire(handle_id_t id,handle_type_t type){
	if (type!=HANDLE_TYPE_ANY&&type!=HANDLE_ID_GET_TYPE(id)){
		return NULL;
	}
	handle_type_data_t* type_data=handle_type_data+HANDLE_ID_GET_TYPE(id);
	lock_acquire_shared(&(type_data->lock));
	handle_t* out=_handle_get_node_from_tree(type_data->rb_root,id);
	if (out){
		handle_acquire(out);
	}
	lock_release_shared(&(type_data->lock));
	return out;
}



void _handle_delete_internal(handle_t* handle){
	if (handle->rc){
		return;
	}
	handle_type_data_t* type_data=handle_type_data+HANDLE_ID_GET_TYPE(handle->id);
	lock_acquire_exclusive(&(type_data->lock));
	_handle_remove_node_from_tree(&(type_data->rb_root),handle);
	handle_type_data->active_count--;
	(handle_type_data+HANDLE_ID_GET_TYPE(handle->id))->active_count--;
	(handle_type_data+HANDLE_ID_GET_TYPE(handle->id))->delete_callback(handle);
	lock_release_exclusive(&(type_data->lock));
}
