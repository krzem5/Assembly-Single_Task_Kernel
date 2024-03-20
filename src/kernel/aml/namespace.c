#include <kernel/aml/namespace.h>
#include <kernel/aml/object.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/types.h>



static omm_allocator_t* _aml_namespace_allocator=NULL;
static aml_namespace_t* _aml_namespace_root=NULL;



KERNEL_PUBLIC aml_namespace_t* aml_namespace_lookup(aml_namespace_t* root,const char* path,u8 flags){
	if (!_aml_namespace_root){
		_aml_namespace_allocator=omm_init("aml_namespace",sizeof(aml_namespace_t),8,2,pmm_alloc_counter("omm_aml_namespace"));
		spinlock_init(&(_aml_namespace_allocator->lock));
		_aml_namespace_root=omm_alloc(_aml_namespace_allocator);
		_aml_namespace_root->name[0]='\\';
		_aml_namespace_root->name[1]=0;
		_aml_namespace_root->value=NULL;
		_aml_namespace_root->parent=_aml_namespace_root;
		_aml_namespace_root->prev_sibling=NULL;
		_aml_namespace_root->next_sibling=NULL;
		_aml_namespace_root->first_child=NULL;
	}
	if (!root){
		root=_aml_namespace_root;
	}
	if (path[0]=='\\'){
		root=_aml_namespace_root;
		path++;
	}
_retry_namespace:
	aml_namespace_t* node=root;
	u32 i=0;
	while (node&&path[i]){
		if (path[i]=='^'){
			node=node->parent;
			i++;
		}
		else if (path[i]=='.'){
			i++;
		}
		else{
			for (aml_namespace_t* child=node->first_child;child;child=child->next_sibling){
				if (*((const u32*)(child->name))==*((const u32*)(path+i))){
					node=child;
					goto _child_found;
				}
			}
			if (!(flags&AML_NAMESPACE_LOOKUP_FLAG_CREATE)){
				node=NULL;
				break;
			}
			aml_namespace_t* child=omm_alloc(_aml_namespace_allocator);
			child->name[0]=path[i];
			child->name[1]=path[i+1];
			child->name[2]=path[i+2];
			child->name[3]=path[i+3];
			child->name[4]=0;
			child->value=NULL;
			child->parent=node;
			child->prev_sibling=NULL;
			child->next_sibling=node->first_child;
			child->first_child=NULL;
			if (node->first_child){
				node->first_child->prev_sibling=child;
			}
			node->first_child=child;
			node=child;
_child_found:
			i+=4;
		}
	}
	if (!node){
		if ((flags&AML_NAMESPACE_LOOKUP_FLAG_LOCAL)||root->parent==root){
			return NULL;
		}
		root=root->parent;
		goto _retry_namespace;
	}
	if ((flags&AML_NAMESPACE_LOOKUP_FLAG_CLEAR)&&node->value){
		aml_object_dealloc(node->value);
		node->value=NULL;
	}
	return node;
}
