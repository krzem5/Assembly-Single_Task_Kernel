#include <core/types.h>
#include <linker/shared_object.h>



static shared_object_t* _shared_object_tail=NULL;

shared_object_t* shared_object_root=NULL;



shared_object_t* shared_object_alloc(u64 image_base){
	static shared_object_t buffer[16];
	static u8 index=0;
	shared_object_t* out=buffer+(index++);
	out->next=NULL;
	out->image_base=image_base;
	if (_shared_object_tail){
		_shared_object_tail->next=out;
	}
	else{
		shared_object_root=out;
	}
	_shared_object_tail=out;
	return out;
}
