#include <kernel/lock/spinlock.h>
#include <kernel/log/log.h>
#include <kernel/tree/rb_tree.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "rb_tree"



static KERNEL_INLINE rb_tree_node_t* _get_parent(rb_tree_node_t* node){
	return (rb_tree_node_t*)(node->rb_parent_and_color&0xfffffffffffffffeull);
}



static KERNEL_INLINE void _set_parent(rb_tree_node_t* node,rb_tree_node_t* parent){
	node->rb_parent_and_color=((u64)parent)|(node->rb_parent_and_color&1);
}



static KERNEL_INLINE _Bool _get_color(rb_tree_node_t* node){
	return node->rb_parent_and_color&1;
}



static KERNEL_INLINE void _set_color(rb_tree_node_t* node,_Bool color){
	node->rb_parent_and_color=(node->rb_parent_and_color&0xfffffffffffffffeull)|color;
}



static void _rotate_subtree(rb_tree_t* tree,rb_tree_node_t* x,_Bool dir){
	rb_tree_node_t* y=_get_parent(x);
	rb_tree_node_t* z=x->rb_nodes[dir^1];
	x->rb_nodes[dir^1]=z->rb_nodes[dir];
	if (z->rb_nodes[dir]){
		_set_parent(z->rb_nodes[dir],x);
	}
	z->rb_nodes[dir]=x;
	_set_parent(x,z);
	_set_parent(z,y);
	if (y){
		y->rb_nodes[x==y->rb_right]=z;
	}
	else{
		tree->root=z;
	}
}



KERNEL_PUBLIC void rb_tree_init(rb_tree_t* tree){
	tree->root=NULL;
	spinlock_init(&(tree->lock));
}



KERNEL_PUBLIC void rb_tree_insert_node(rb_tree_t* tree,rb_tree_node_t* x){
	spinlock_acquire_exclusive(&(tree->lock));
	x->rb_left=NULL;
	x->rb_right=NULL;
	if (!tree->root){
		x->rb_parent_and_color=0;
		tree->root=x;
		goto _cleanup;
	}
	x->rb_parent_and_color=1;
	rb_tree_node_t* y=tree->root;
	while (y->rb_nodes[y->key<x->key]){
		if (x->key==y->key){
			panic("rb_tree_insert_node: duplicated key");
		}
		y=y->rb_nodes[y->key<x->key];
	}
	if (x->key==y->key){
		panic("rb_tree_insert_node: duplicated key");
	}
	_set_parent(x,y);
	y->rb_nodes[y->key<x->key]=x;
	while (y&&_get_color(y)){
		rb_tree_node_t* z=_get_parent(y);
		if (!z){
			_set_color(y,0);
			break;
		}
		_Bool dir=(y==z->rb_right);
		rb_tree_node_t* w=z->rb_nodes[dir^1];
		if (!w||!_get_color(w)){
			if (x==y->rb_nodes[dir^1]){
				_rotate_subtree(tree,y,dir);
				y=z->rb_nodes[dir];
			}
			_rotate_subtree(tree,z,dir^1);
			_set_color(y,0);
			_set_color(z,1);
			break;
		}
		_set_color(y,0);
		_set_color(z,1);
		_set_color(w,0);
		x=z;
		y=_get_parent(x);
	}
_cleanup:
	spinlock_release_exclusive(&(tree->lock));
}



KERNEL_PUBLIC rb_tree_node_t* rb_tree_lookup_node(rb_tree_t* tree,u64 key){
	spinlock_acquire_shared(&(tree->lock));
	for (rb_tree_node_t* x=tree->root;x;x=x->rb_nodes[x->key<key]){
		if (x->key==key){
			spinlock_release_shared(&(tree->lock));
			return x;
		}
	}
	spinlock_release_shared(&(tree->lock));
	return NULL;
}



KERNEL_PUBLIC rb_tree_node_t* rb_tree_lookup_decreasing_node(rb_tree_t* tree,u64 key){
	spinlock_acquire_shared(&(tree->lock));
	rb_tree_node_t* x=tree->root;
	while (x&&x->key!=key){
		rb_tree_node_t* y=x->rb_nodes[x->key<key];
		if (y){
			x=y;
			continue;
		}
		if (x->key<key){
			break;
		}
		if (x->rb_left){
			for (x=x->rb_left;x->rb_right;x=x->rb_right);
			break;
		}
		do{
			y=x;
			x=_get_parent(x);
		} while (x&&y==x->rb_left);
		break;
	}
	spinlock_release_shared(&(tree->lock));
	return x;
}



KERNEL_PUBLIC rb_tree_node_t* rb_tree_lookup_increasing_node(rb_tree_t* tree,u64 key){
	spinlock_acquire_shared(&(tree->lock));
	rb_tree_node_t* x=tree->root;
	while (x&&x->key!=key){
		rb_tree_node_t* y=x->rb_nodes[x->key<key];
		if (y){
			x=y;
			continue;
		}
		if (x->key>key){
			break;
		}
		if (x->rb_right){
			for (x=x->rb_right;x->rb_left;x=x->rb_left);
			break;
		}
		do{
			y=x;
			x=_get_parent(x);
		} while (x&&y==x->rb_right);
		break;
	}
	spinlock_release_shared(&(tree->lock));
	return x;
}



KERNEL_PUBLIC void rb_tree_remove_node(rb_tree_t* tree,rb_tree_node_t* x){
	spinlock_acquire_exclusive(&(tree->lock));
	rb_tree_node_t* y=_get_parent(x);
	if (x->rb_left&&x->rb_right){
		rb_tree_node_t* z=x->rb_right;
		for (;z->rb_left;z=z->rb_left);
		if (y){
			y->rb_nodes[x==y->rb_right]=z;
		}
		else{
			tree->root=z;
		}
		z->rb_left=x->rb_left;
		x->rb_left=NULL;
		_set_parent(z->rb_left,z);
		if (z==x->rb_right){
			x->rb_right=z->rb_right;
			z->rb_right=x;
			_set_parent(x,z);
		}
		else{
			rb_tree_node_t* w=x->rb_right;
			x->rb_right=z->rb_right;
			z->rb_right=w;
			_set_parent(z->rb_right,z);
			w=_get_parent(z);
			w->rb_nodes[z==w->rb_right]=x;
			_set_parent(x,w);
		}
		if (x->rb_right){
			_set_parent(x->rb_right,x);
		}
		_set_parent(z,y);
		y=_get_parent(x);
		_Bool tmp=_get_color(z);
		_set_color(z,_get_color(x));
		_set_color(x,tmp);
	}
	if (x->rb_left||x->rb_right){
		rb_tree_node_t* z=(x->rb_left?x->rb_left:x->rb_right);
		if (y){
			y->rb_nodes[x==y->rb_right]=z;
		}
		else{
			tree->root=z;
		}
		_set_color(z,0);
		_set_parent(z,y);
		goto _cleanup;
	}
	if (x==tree->root){
		tree->root=NULL;
		goto _cleanup;
	}
	_Bool dir=(x==y->rb_right);
	y->rb_nodes[dir]=NULL;
	if (_get_color(x)){
		goto _cleanup;
	}
	while (1){
		rb_tree_node_t* z=y->rb_nodes[dir^1];
		rb_tree_node_t* u=z->rb_nodes[dir^1];
		rb_tree_node_t* v=z->rb_nodes[dir];
		if (_get_color(z)){
			_rotate_subtree(tree,y,dir);
			_set_color(y,1);
			_set_color(z,0);
			z=v;
			u=z->rb_nodes[dir^1];
			v=z->rb_nodes[dir];
		}
		if (u&&_get_color(u)){
			_rotate_subtree(tree,y,dir);
			_set_color(z,_get_color(y));
			_set_color(y,0);
			_set_color(u,0);
			break;
		}
		if (v&&_get_color(v)){
			_rotate_subtree(tree,z,dir^1);
			_rotate_subtree(tree,y,dir);
			_set_color(v,_get_color(y));
			_set_color(y,0);
			_set_color(z,0);
			break;
		}
		_set_color(z,1);
		if (_get_color(y)){
			_set_color(y,0);
			break;
		}
		x=y;
		y=_get_parent(x);
		if (!y){
			break;
		}
		dir=(x==y->rb_right);
	}
_cleanup:
	spinlock_release_exclusive(&(tree->lock));
}



KERNEL_PUBLIC rb_tree_node_t* rb_tree_iter_start(rb_tree_t* tree){
	if (!tree->root){
		return NULL;
	}
	spinlock_acquire_shared(&(tree->lock));
	rb_tree_node_t* x=tree->root;
	for (;x->rb_left;x=x->rb_left);
	spinlock_release_shared(&(tree->lock));
	return x;
}



KERNEL_PUBLIC rb_tree_node_t* rb_tree_iter_next(rb_tree_t* tree,rb_tree_node_t* x){
	spinlock_acquire_shared(&(tree->lock));
	if (x->rb_right){
		for (x=x->rb_right;x->rb_left;x=x->rb_left);
	}
	else{
		rb_tree_node_t* y;
		do{
			y=x;
			x=_get_parent(x);
		} while (x&&y==x->rb_right);
	}
	spinlock_release_shared(&(tree->lock));
	return x;
}
