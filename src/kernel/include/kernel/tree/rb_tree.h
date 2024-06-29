#ifndef _KERNEL_RB_TREE_H_
#define _KERNEL_RB_TREE_H_ 1
#include <kernel/lock/rwlock.h>
#include <kernel/types.h>



typedef struct _RB_TREE_NODE{
	u64 key;
	u64 rb_parent_and_color;
	union{
		struct{
			struct _RB_TREE_NODE* rb_left;
			struct _RB_TREE_NODE* rb_right;
		};
		struct _RB_TREE_NODE* rb_nodes[2];
	};
} rb_tree_node_t;



typedef struct _RB_TREE{
	rb_tree_node_t* root;
	rwlock_t lock;
} rb_tree_t;



void rb_tree_init(rb_tree_t* tree);



void rb_tree_insert_node(rb_tree_t* tree,rb_tree_node_t* x);



rb_tree_node_t* rb_tree_lookup_or_insert_node(rb_tree_t* tree,rb_tree_node_t* x); // if successful, this does not release the tree lock



rb_tree_node_t* rb_tree_lookup_node(rb_tree_t* tree,u64 key);



rb_tree_node_t* rb_tree_lookup_decreasing_node(rb_tree_t* tree,u64 key);



rb_tree_node_t* rb_tree_lookup_increasing_node(rb_tree_t* tree,u64 key);



void rb_tree_remove_node(rb_tree_t* tree,rb_tree_node_t* x);



rb_tree_node_t* rb_tree_iter_start(rb_tree_t* tree);



rb_tree_node_t* rb_tree_iter_next(rb_tree_t* tree,rb_tree_node_t* x);



#endif
