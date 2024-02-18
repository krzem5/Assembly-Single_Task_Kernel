#include <kernel/lock/spinlock.h>
#include <kernel/log/log.h>
#include <kernel/tree/rb_tree.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "rb_tree"



#define NIL_NODE (&_rb_tree_nil_node)



static rb_tree_node_t _rb_tree_nil_node={
	.rb_parent_and_color=0,
	.rb_left=NULL,
	.rb_right=NULL
};



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



static KERNEL_INLINE void _replace_node(rb_tree_t* tree,rb_tree_node_t* old,rb_tree_node_t* new){
	rb_tree_node_t* parent=_get_parent(old);
	if (parent){
		parent->rb_nodes[parent->rb_right==old]=new;
	}
	else{
		tree->root=new;
	}
}



KERNEL_PUBLIC void rb_tree_init(rb_tree_t* tree){
	tree->root=NIL_NODE;
	spinlock_init(&(tree->lock));
}



static rb_tree_node_t* RotateDirRoot(rb_tree_t* tree,rb_tree_node_t* P,_Bool dir){
	rb_tree_node_t* G=_get_parent(P);
	rb_tree_node_t* S=P->rb_nodes[dir^1];
	rb_tree_node_t* C=S->rb_nodes[dir];
	P->rb_nodes[dir^1]=C;
	if (C!=NIL_NODE){
		_set_parent(C,P);
	}
	S->rb_nodes[dir]=P;
	_set_parent(P,S);
	_set_parent(S,G);
	if (G){
		G->rb_nodes[P==G->rb_right]=S;
	}
	else{
		tree->root=S;
	}
	return S;
}



static void _print_tree_recursive(rb_tree_node_t* node,u32 indent){
	if (node==NIL_NODE){
		return;
	}
	_print_tree_recursive(node->rb_left,indent+1);
	for (u32 i=0;i<indent;i++){
		log("  ");
	}
	log("%lu (%c) => %p:%lu\n",node->key,(_get_color(node)?'R':'B'),_get_parent(node),(_get_parent(node)?_get_parent(node)->key:0));
	_print_tree_recursive(node->rb_right,indent+1);
}



KERNEL_PUBLIC void rb_tree_insert_node(rb_tree_t* tree,rb_tree_node_t* x){
	spinlock_acquire_exclusive(&(tree->lock));
	// WARN("Insert: %u",x->key);
	x->rb_left=NIL_NODE;
	x->rb_right=NIL_NODE;
	if (tree->root==NIL_NODE){
		x->rb_parent_and_color=0;
		tree->root=x;
		goto _cleanup;
	}
	x->rb_parent_and_color=1;
	rb_tree_node_t* y=tree->root;
	while (y->rb_nodes[y->key<x->key]!=NIL_NODE){
		if (x->key==y->key){
			WARN("%u",x->key);
			_print_tree_recursive(tree->root,0);
			panic("rb_tree_insert_node: duplicated key");
		}
		y=y->rb_nodes[y->key<x->key];
	}
	if (x->key==y->key){
		WARN("%u",x->key);
		_print_tree_recursive(tree->root,0);
		panic("rb_tree_insert_node: duplicated key");
	}
	_set_parent(x,y);
	y->rb_nodes[y->key<x->key]=x;
	/*********************************************/
	rb_tree_node_t* z=NULL;
	rb_tree_node_t* w=NULL;
	do{
		if (!_get_color(y)){
			break;
		}
		if (!(z=_get_parent(y))){
			_set_color(y,0);
			break;
		}
		_Bool dir=(y==z->rb_right);
		w=z->rb_nodes[dir^1];
		if (w==NIL_NODE||!_get_color(w)){
			if (x==y->rb_nodes[dir^1]){
				RotateDirRoot(tree,y,dir);
				x=y;
				y=z->rb_nodes[dir];
			}
			RotateDirRoot(tree,z,dir^1);
			_set_color(y,0);
			_set_color(z,1);
			break;
		}
		_set_color(y,0);
		_set_color(z,1);
		_set_color(w,0);
		x=z;
	} while ((y=_get_parent(x)));
	/*********************************************/
	// if (y==tree->root){
	// 	goto _cleanup;
	// }
	// while (x!=tree->root&&_get_color(_get_parent(x))){
	// 	y=_get_parent(x);
	// 	rb_tree_node_t* z=_get_parent(y);
	// 	rb_tree_node_t* w=z->rb_nodes[y==z->rb_left];
	// 	if (_get_color(w)){
	// 		_set_color(y,0);
	// 		_set_color(z,1);
	// 		_set_color(w,0);
	// 		x=z;
	// 		continue;
	// 	}
	// 	_Bool i=(y==z->rb_left);
	// 	if (x==y->rb_nodes[i]){
	// 		x=y;
	// 		y=x->rb_nodes[i];
	// 		w=y->rb_nodes[i^1];
	// 		x->rb_nodes[i]=w;
	// 		y->rb_nodes[i^1]=x;
	// 		_set_parent(x,y);
	// 		_set_parent(y,z);
	// 		_set_parent(w,x);
	// 		z->rb_nodes[x==z->rb_right]=y;
	// 	}
	// 	_set_color(y,0);
	// 	_set_color(z,1);
	// 	y=z->rb_nodes[i^1];
	// 	w=y->rb_nodes[i];
	// 	z->rb_nodes[i^1]=w;
	// 	y->rb_nodes[i]=z;
	// 	_set_parent(y,_get_parent(z));
	// 	_set_parent(w,z);
	// 	if (!_get_parent(z)){
	// 		tree->root=y;
	// 	}
	// 	else{
	// 		_get_parent(z)->rb_nodes[z==_get_parent(z)->rb_right]=y;
	// 	}
	// 	_set_parent(z,y);
	// }
	// tree->root->rb_parent_and_color=0;
_cleanup:
	spinlock_release_exclusive(&(tree->lock));
}



KERNEL_PUBLIC void rb_tree_insert_node_increasing(rb_tree_t* tree,rb_tree_node_t* x){
	rb_tree_insert_node(tree,x);
// 	spinlock_acquire_exclusive(&(tree->lock));
// 	x->rb_left=NIL_NODE;
// 	x->rb_right=NIL_NODE;
// 	if (tree->root==NIL_NODE){
// 		x->rb_parent_and_color=0;
// 		tree->root=x;
// 		goto _cleanup;
// 	}
// 	x->rb_parent_and_color=1;
// 	rb_tree_node_t* y=tree->root;
// 	for (;y->rb_right!=NIL_NODE;y=y->rb_right);
// 	_set_parent(x,y);
// 	y->rb_right=x;
// 	if (!_get_parent(y)){
// 		goto _cleanup;
// 	}
// 	while (1){
// 		y=_get_parent(x);
// 		if (!y||!_get_color(y)){
// 			break;
// 		}
// 		_set_color(y,0);
// 		y=_get_parent(y);
// 		_set_color(y,1);
// 		rb_tree_node_t* z=y->rb_left;
// 		if (_get_color(z)){
// 			_set_color(z,0);
// 			x=y;
// 			continue;
// 		}
// 		z=y->rb_right;
// 		y->rb_right=z->rb_left;
// 		_set_parent(z->rb_left,y);
// 		_set_parent(z,_get_parent(y));
// 		if (_get_parent(y)){
// 			_get_parent(y)->rb_right=z;
// 		}
// 		else{
// 			tree->root=z;
// 		}
// 		z->rb_left=y;
// 		_set_parent(y,z);
// 	}
// 	_set_color(tree->root,0);
// _cleanup:
// 	spinlock_release_exclusive(&(tree->lock));
}



KERNEL_PUBLIC rb_tree_node_t* rb_tree_lookup_node(rb_tree_t* tree,u64 key){
	spinlock_acquire_shared(&(tree->lock));
	for (rb_tree_node_t* x=tree->root;x!=NIL_NODE;x=x->rb_nodes[x->key<key]){
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
	while (x!=NIL_NODE&&x->key!=key){
		rb_tree_node_t* y=x->rb_nodes[x->key<key];
		if (y!=NIL_NODE){
			x=y;
			continue;
		}
		if (x->key<key){
			break;
		}
		if (x->rb_left!=NIL_NODE){
			for (x=x->rb_left;x->rb_right!=NIL_NODE;x=x->rb_right);
			break;
		}
		do{
			y=x;
			x=_get_parent(x);
		} while (x&&y==x->rb_left);
		break;
	}
	spinlock_release_shared(&(tree->lock));
	return (x==NIL_NODE?NULL:x);
}



KERNEL_PUBLIC rb_tree_node_t* rb_tree_lookup_increasing_node(rb_tree_t* tree,u64 key){
	spinlock_acquire_shared(&(tree->lock));
	rb_tree_node_t* x=tree->root;
	while (x!=NIL_NODE&&x->key!=key){
		rb_tree_node_t* y=x->rb_nodes[x->key<key];
		if (y!=NIL_NODE){
			x=y;
			continue;
		}
		if (x->key>key){
			break;
		}
		if (x->rb_right!=NIL_NODE){
			for (x=x->rb_right;x->rb_left!=NIL_NODE;x=x->rb_left);
			break;
		}
		do{
			y=x;
			x=_get_parent(x);
		} while (x&&y==x->rb_right);
		break;
	}
	spinlock_release_shared(&(tree->lock));
	return (x==NIL_NODE?NULL:x);
}



static _Bool _search_for_key(rb_tree_node_t* node,u64 key){
	return node!=NIL_NODE&&(node->key==key||_search_for_key(node->rb_left,key)||_search_for_key(node->rb_right,key));
}



KERNEL_PUBLIC void rb_tree_remove_node(rb_tree_t* tree,rb_tree_node_t* N){
	spinlock_acquire_exclusive(&(tree->lock));
	/*********************************************/
	u64 original_key=N->key;
	// WARN("Remove: %u",original_key);
	rb_tree_node_t* P=_get_parent(N);
	if (N->rb_left!=NIL_NODE&&N->rb_right!=NIL_NODE){
		rb_tree_node_t* successor=N->rb_right;
		for (;successor->rb_left!=NIL_NODE;successor=successor->rb_left);
		// WARN("Remove: N: %u, P: %u, successor: %u",N->key,P->key,successor->key);
		if (successor==N->rb_right){
			if (P){
				P->rb_nodes[N==P->rb_right]=successor;
			}
			else{
				tree->root=successor;
			}
			successor->rb_left=N->rb_left;
			N->rb_left=NIL_NODE;
			_set_parent(successor->rb_left,successor);
			N->rb_right=successor->rb_right;
			successor->rb_right=N;
			if (N->rb_right){
				_set_parent(N->rb_right,N);
			}
			_set_parent(N,successor);
			_set_parent(successor,P);
			P=successor;
			_Bool is_successor_red=_get_color(successor);
			_set_color(successor,_get_color(N));
			_set_color(N,is_successor_red);
			// _print_tree_recursive(tree->root,0);
			// WARN("Case 1a");
			// WARN("Remove: N: %u, P: %u",N->key,P->key);
			// panic("Case 1a");
		}
		else{
			// WARN("Remove: N: %u, P: %u, successor: %u",N->key,P->key,successor->key);
			// _print_tree_recursive(tree->root,0);
			if (P){
				P->rb_nodes[N==P->rb_right]=successor;
			}
			else{
				tree->root=successor;
			}
			successor->rb_left=N->rb_left;
			N->rb_left=NIL_NODE;
			_set_parent(successor->rb_left,successor);
			rb_tree_node_t* tmp=N->rb_right;
			N->rb_right=successor->rb_right;
			successor->rb_right=tmp;
			if (N->rb_right){
				_set_parent(N->rb_right,N);
			}
			_set_parent(successor->rb_right,successor);
			rb_tree_node_t* successor_parent=_get_parent(successor);
			successor_parent->rb_nodes[successor==successor_parent->rb_right]=N;
			_set_parent(N,successor_parent);
			_set_parent(successor,P);
			P=_get_parent(N);
			_Bool is_successor_red=_get_color(successor);
			_set_color(successor,_get_color(N));
			_set_color(N,is_successor_red);
			// WARN("Case 1b");
			// _print_tree_recursive(tree->root,0);
			// panic("Case 1b");
		}
	}
	if (N->rb_left!=NIL_NODE||N->rb_right!=NIL_NODE){
		rb_tree_node_t* child=(N->rb_left!=NIL_NODE?N->rb_left:N->rb_right);
		if (P){
			P->rb_nodes[N==P->rb_right]=child;
		}
		else{
			tree->root=child;
		}
		_set_color(child,0);
		_set_parent(child,P);
		// panic("Case 2");
		goto _cleanup;
	}
	if (N==tree->root){
		tree->root=NIL_NODE;
		goto _cleanup;
	}
	if (_get_color(N)){
		P->rb_nodes[N==P->rb_right]=NIL_NODE;
		goto _cleanup;
	}
	_Bool dir=(N==P->rb_right);
	P->rb_nodes[dir]=NIL_NODE;
	rb_tree_node_t* S;
	rb_tree_node_t* C;
	rb_tree_node_t* D;
	goto _skip_first_dir;
	do{
		dir=(N==P->rb_right);
_skip_first_dir:
		S=P->rb_nodes[dir^1];
		D=S->rb_nodes[dir^1];
		C=S->rb_nodes[dir];
		if (_get_color(S)){
			goto Case_D3;
		}
		if (D!=NIL_NODE&&_get_color(D)){
			goto Case_D6;
		}
		if (C!=NIL_NODE&&_get_color(C)){
			goto Case_D5;
		}
		if (_get_color(P)){
			goto Case_D4;
		}
		_set_color(S,1);
		N=P;
	} while ((P=_get_parent(N)));
	goto _cleanup;
Case_D3:
	RotateDirRoot(tree,P,dir);
	_set_color(P,1);
	_set_color(S,0);
	S=C;
	D=S->rb_nodes[dir^1];
	if (D!=NIL_NODE&&_get_color(D)){
		goto Case_D6;
	}
	C=S->rb_nodes[dir];
	if (C!=NIL_NODE&&_get_color(C)){
		goto Case_D5;
	}
Case_D4:
	_set_color(S,1);
	_set_color(P,0);
	goto _cleanup;
Case_D5:
	RotateDirRoot(tree,S,dir^1);
	_set_color(S,1);
	_set_color(C,0);
	D=S;
	S=C;
Case_D6:
	RotateDirRoot(tree,P,dir);
	_set_color(S,_get_color(P));
	_set_color(P,0);
	_set_color(D,0);
	/*********************************************/
	// _Bool skip_recursive_fix=_get_color(x);
	// rb_tree_node_t* z;
	// rb_tree_node_t* z_parent;
	// if (x->rb_left!=NIL_NODE&&x->rb_right!=NIL_NODE){
	// 	rb_tree_node_t* y=x->rb_right;
	// 	for (;y->rb_left!=NIL_NODE;y=y->rb_left);
	// 	skip_recursive_fix=_get_color(y);
	// 	z=y->rb_right;
	// 	z_parent=y;
	// 	if (_get_parent(y)!=x){
	// 		_get_parent(y)->rb_left=y->rb_right;
	// 		y->rb_right=x->rb_right;
	// 		_set_parent(x->rb_right,y);
	// 	}
	// 	_replace_node(tree,x,y);
	// 	y->rb_parent_and_color=x->rb_parent_and_color;
	// 	y->rb_left=x->rb_left;
	// 	_set_parent(y->rb_left,y);
	// }
	// else{
	// 	z=x->rb_nodes[x->rb_left==NIL_NODE];
	// 	_replace_node(tree,x,z);
	// 	z_parent=_get_parent(x);
	// 	_set_parent(z,z_parent);
	// }
	// if (skip_recursive_fix){
	// 	goto _cleanup;
	// }
	// while (z_parent&&!_get_color(z)){
	// 	_Bool i=(z==z_parent->rb_left);
	// 	x=z_parent->rb_nodes[i];
	// 	if (_get_color(x)){
	// 		_set_color(x,0);
	// 		_set_color(z_parent,1);
	// 		rb_tree_node_t* y=x->rb_nodes[!i];
	// 		z_parent->rb_nodes[i]=y;
	// 		_set_parent(y,z_parent);
	// 		_set_parent(x,_get_parent(z_parent));
	// 		_replace_node(tree,z_parent,x);
	// 		x->rb_nodes[!i]=z_parent;
	// 		_set_parent(z_parent,x);
	// 		x=y;
	// 	}
	// 	if (!_get_color(x->rb_left)&&!_get_color(x->rb_right)){
	// 		_set_color(x,1);
	// 		z=z_parent;
	// 		z_parent=_get_parent(z);
	// 		continue;
	// 	}
	// 	if (!_get_color(x->rb_nodes[i])){
	// 		_set_color(x->rb_nodes[!i],0);
	// 		_set_color(x,1);
	// 		rb_tree_node_t* y=x->rb_nodes[!i];
	// 		x->rb_nodes[!i]=y->rb_nodes[i];
	// 		_set_parent(y->rb_nodes[i],x);
	// 		_set_parent(y,_get_parent(x));
	// 		_get_parent(x)->rb_nodes[x==_get_parent(x)->rb_right]=y;
	// 		y->rb_nodes[i]=x;
	// 		_set_parent(x,y);
	// 		x=z_parent->rb_nodes[i];
	// 	}
	// 	_set_color(x,_get_color(z_parent));
	// 	_set_color(z_parent,0);
	// 	_set_color(x->rb_nodes[i],0);
	// 	z_parent->rb_nodes[i]=x->rb_nodes[!i];
	// 	_set_parent(x->rb_nodes[!i],z_parent);
	// 	_set_parent(x,_get_parent(z_parent));
	// 	_replace_node(tree,z_parent,x);
	// 	x->rb_nodes[!i]=z_parent;
	// 	_set_parent(z_parent,x);
	// 	_set_color(tree->root,0);
	// 	goto _cleanup;
	// }
	// _set_color(z,0);
_cleanup:
	if (_search_for_key(tree->root,original_key)){
		WARN("post-remove: %u",original_key);
		_print_tree_recursive(tree->root,0);
		panic("AAA");
	}
	spinlock_release_exclusive(&(tree->lock));
}



KERNEL_PUBLIC rb_tree_node_t* rb_tree_iter_start(rb_tree_t* tree){
	if (tree->root==NIL_NODE){
		return NULL;
	}
	spinlock_acquire_shared(&(tree->lock));
	rb_tree_node_t* x=tree->root;
	for (;x->rb_left!=NIL_NODE;x=x->rb_left);
	spinlock_release_shared(&(tree->lock));
	return x;
}



KERNEL_PUBLIC rb_tree_node_t* rb_tree_iter_next(rb_tree_t* tree,rb_tree_node_t* x){
	spinlock_acquire_shared(&(tree->lock));
	if (x->rb_right!=NIL_NODE){
		for (x=x->rb_right;x->rb_left!=NIL_NODE;x=x->rb_left);
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
