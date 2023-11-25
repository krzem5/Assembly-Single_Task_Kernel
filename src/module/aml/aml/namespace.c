#include <aml/namespace.h>
#include <aml/object.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/types.h>



static omm_allocator_t* _aml_namespace_allocator=NULL;
static aml_namespace_t* _aml_namespace_root=NULL;



KERNEL_PUBLIC aml_namespace_t* aml_namespace_lookup(aml_namespace_t* root,const char* path,u8 flags){
	if (!_aml_namespace_root){
		_aml_namespace_allocator=omm_init("aml_namespace",sizeof(aml_namespace_t),8,2,pmm_alloc_counter("omm_aml_namespace"));
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
	while (root&&path[0]){
		if (path[0]=='^'){
			root=root->parent;
			path++;
		}
		else if (path[0]=='.'){
			path++;
		}
		else{
			for (aml_namespace_t* child=root->first_child;child;child=child->next_sibling){
				if (*((const u32*)(child->name))==*((const u32*)path)){
					root=child;
					goto _child_found;
				}
			}
			if (!(flags&AML_NAMESPACE_LOOKUP_FLAG_CREATE)){
				return NULL;
			}
			aml_namespace_t* child=omm_alloc(_aml_namespace_allocator);
			child->name[0]=path[0];
			child->name[1]=path[1];
			child->name[2]=path[2];
			child->name[3]=path[3];
			child->name[4]=0;
			child->value=NULL;
			child->parent=root;
			child->prev_sibling=NULL;
			child->next_sibling=root->first_child;
			child->first_child=NULL;
			if (root->first_child){
				root->first_child->prev_sibling=child;
			}
			root->first_child=child;
			root=child;
_child_found:
			path+=4;
		}
	}
	if ((flags&AML_NAMESPACE_LOOKUP_FLAG_CLEAR)&&root&&root->value){
		aml_object_dealloc(root->value);
		root->value=0;
	}
	return root;
}
