#include <kernel/handle/handle.h>
#include <kernel/kernel.h>
#include <kernel/lock/lock.h>
#include <kernel/log/log.h>
#include <kernel/memory/kmm.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "handle"



#define RB_NIL_NODE (&_handle_rb_nil_node)



static handle_t _handle_rb_nil_node={
	.rb_parent_and_color=0,
	.rb_left=NULL,
	.rb_right=NULL
};

handle_type_data_t* handle_type_data;
handle_type_t handle_type_count;



static KERNEL_INLINE handle_t* _rb_get_parent(handle_t* node){
	return (handle_t*)(node->rb_parent_and_color&0xfffffffffffffffeull);
}



static KERNEL_INLINE void _rb_set_parent(handle_t* node,handle_t* parent){
	node->rb_parent_and_color=((u64)parent)|(node->rb_parent_and_color&1);
}



static KERNEL_INLINE _Bool _rb_get_color(handle_t* node){
	return node->rb_parent_and_color&1;
}



static KERNEL_INLINE void _rb_set_color(handle_t* node,_Bool color){
	node->rb_parent_and_color=(node->rb_parent_and_color&0xfffffffffffffffeull)|color;
}



static KERNEL_INLINE void _replace_tree_node(handle_t** root,handle_t* old,handle_t* new){
	handle_t* parent=_rb_get_parent(old);
	if (parent){
		parent->rb_nodes[parent->rb_right==old]=new;
	}
	else{
		*root=new;
	}
}



static void _handle_add_node_to_tree(handle_t** root,handle_t* x){
	x->rb_left=RB_NIL_NODE;
	x->rb_right=RB_NIL_NODE;
	if (*root==RB_NIL_NODE){
		x->rb_parent_and_color=0;
		*root=x;
		return;
	}
	_rb_set_color(x,1);
	handle_t* y=*root;
	for (;y->rb_right!=RB_NIL_NODE;y=y->rb_right);
	_rb_set_parent(x,y);
	y->rb_right=x;
	if (!_rb_get_parent(y)){
		return;
	}
	while (1){
		y=_rb_get_parent(x);
		if (!y||!_rb_get_color(y)){
			break;
		}
		_rb_set_color(y,0);
		y=_rb_get_parent(y);
		_rb_set_color(y,1);
		handle_t* z=y->rb_left;
		if (_rb_get_color(z)){
			_rb_set_color(z,0);
			x=y;
			continue;
		}
		z=y->rb_right;
		y->rb_right=z->rb_left;
		_rb_set_parent(z->rb_left,y);
		_rb_set_parent(z,_rb_get_parent(y));
		if (_rb_get_parent(y)){
			_rb_get_parent(y)->rb_right=z;
		}
		else{
			*root=z;
		}
		z->rb_left=y;
		_rb_set_parent(y,z);
	}
	_rb_set_color(*root,0);
}



static handle_t* _handle_get_node_from_tree(handle_t* root,handle_id_t id){
	for (handle_t* x=root;x!=RB_NIL_NODE;x=x->rb_nodes[x->id<id]){
		if (x->id==id){
			return x;
		}
	}
	return NULL;
}



static void _handle_remove_node_from_tree(handle_t** root,handle_t* x){
	_Bool skip_recursive_fix=_rb_get_color(x);
	handle_t* z;
	handle_t* z_parent;
	if (x->rb_left!=RB_NIL_NODE&&x->rb_right!=RB_NIL_NODE){
		handle_t* y=x->rb_right;
		for (;y->rb_left!=RB_NIL_NODE;y=y->rb_left);
		skip_recursive_fix=_rb_get_color(y);
		z=y->rb_right;
		z_parent=y;
		if (_rb_get_parent(y)!=x){
			_rb_get_parent(y)->rb_left=y->rb_right;
			y->rb_right=x->rb_right;
			_rb_set_parent(x->rb_right,y);
		}
		_replace_tree_node(root,x,y);
		y->rb_parent_and_color=x->rb_parent_and_color;
		y->rb_left=x->rb_left;
		_rb_set_parent(y->rb_left,y);
	}
	else{
		z=x->rb_nodes[x->rb_left==RB_NIL_NODE];
		_replace_tree_node(root,x,z);
		z_parent=_rb_get_parent(x);
		_rb_set_parent(z,z_parent);
	}
	if (skip_recursive_fix){
		return;
	}
	while (z_parent&&!_rb_get_color(z)){
		_Bool i=(z==z_parent->rb_left);
		x=z_parent->rb_nodes[i];
		if (_rb_get_color(x)){
			_rb_set_color(x,0);
			_rb_set_color(z_parent,1);
			handle_t* y=x->rb_nodes[!i];
			z_parent->rb_nodes[i]=y;
			_rb_set_parent(y,z_parent);
			_rb_set_parent(x,_rb_get_parent(z_parent));
			_replace_tree_node(root,z_parent,x);
			x->rb_nodes[!i]=z_parent;
			_rb_set_parent(z_parent,x);
			x=y;
		}
		if (!_rb_get_color(x->rb_left)&&!_rb_get_color(x->rb_right)){
			_rb_set_color(x,1);
			z=z_parent;
			z_parent=_rb_get_parent(z);
			continue;
		}
		if (!_rb_get_color(x->rb_nodes[i])){
			_rb_set_color(x->rb_nodes[!i],0);
			_rb_set_color(x,1);
			handle_t* y=x->rb_nodes[!i];
			x->rb_nodes[!i]=y->rb_nodes[i];
			_rb_set_parent(y->rb_nodes[i],x);
			_rb_set_parent(y,_rb_get_parent(x));
			_rb_get_parent(x)->rb_nodes[x==_rb_get_parent(x)->rb_right]=y;
			y->rb_nodes[i]=x;
			_rb_set_parent(x,y);
			x=z_parent->rb_nodes[i];
		}
		_rb_set_color(x,_rb_get_color(z_parent));
		_rb_set_color(z_parent,0);
		_rb_set_color(x->rb_nodes[i],0);
		z_parent->rb_nodes[i]=x->rb_nodes[!i];
		_rb_set_parent(x->rb_nodes[!i],z_parent);
		_rb_set_parent(x,_rb_get_parent(z_parent));
		_replace_tree_node(root,z_parent,x);
		x->rb_nodes[!i]=z_parent;
		_rb_set_parent(z_parent,x);
		_rb_set_color(*root,0);
		return;
	}
	_rb_set_color(z,0);
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
	handle_type_data->rb_root=RB_NIL_NODE;
	handle_type_data->count=0;
	handle_type_data->active_count=0;
	for (const handle_descriptor_t*const* descriptor=(void*)kernel_section_handle_start();(u64)descriptor<kernel_section_handle_end();descriptor++){
		if (*descriptor){
			handle_type_data_t* type_data=handle_type_data+(*((*descriptor)->var));
			memcpy_lowercase(type_data->name,(*descriptor)->name,HANDLE_NAME_LENGTH);
			lock_init(&(type_data->lock));
			type_data->delete_callback=(*descriptor)->delete_callback;
			type_data->rb_root=RB_NIL_NODE;
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
