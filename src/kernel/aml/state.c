#include <kernel/aml/aml.h>
#include <kernel/kernel.h>
#include <kernel/log/log.h>
#include <kernel/memory/kmm.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "aml"



#define MAKE_OPCODE(a,b) ((a)|((b)<<8))



typedef struct _RUNTIME_GLOBAL_STATE{
	aml_node_t* root_namespace;
} runtime_global_state_t;



typedef struct _RUNTIME_LOCAL_STATE{
	aml_node_t* namespace;
	u64 args[7];
	u64 locals[8];
	aml_node_t simple_return_value;
} runtime_local_state_t;



aml_node_t* aml_root_node;



static aml_node_t* _execute(runtime_global_state_t* global,runtime_local_state_t* local,const aml_object_t* object);



static void _execute_multiple(runtime_global_state_t* global,runtime_local_state_t* local,const aml_object_t* objects,u32 object_count);



static aml_node_t* _alloc_node(const char* name,u8 type,aml_node_t* parent){
	aml_node_t* out=kmm_allocate(sizeof(aml_node_t));
	if (name){
		for (u8 i=0;i<4;i++){
			out->name[i]=name[i];
		}
	}
	else{
		for (u8 i=0;i<4;i++){
			out->name[i]='?';
		}
	}
	out->name[4]=0;
	out->type=type;
	out->parent=(parent?parent:out);
	out->child=NULL;
	if (parent){
		out->next=parent->child;
		parent->child=out;
	}
	else{
		out->next=NULL;
	}
	return out;
}



static aml_node_t* _get_node(runtime_global_state_t* global,runtime_local_state_t* local,const char* namespace,u8 type){
	aml_node_t* out=local->namespace;
	if (namespace[0]=='\\'){
		out=global->root_namespace;
		namespace++;
	}
	while (namespace[0]){
		if (namespace[0]=='^'){
			out=out->parent;
			namespace++;
		}
		else if (namespace[0]=='.'){
			namespace++;
		}
		else{
			for (aml_node_t* child=out->child;child;child=child->next){
				if (*((const u32*)(child->name))==*((const u32*)namespace)){
					out=child;
					goto _namespace_found;
				}
			}
			out=_alloc_node(namespace,(namespace[4]?AML_NODE_TYPE_SCOPE:type),out);
_namespace_found:
			namespace+=4;
		}
	}
	return out;
}



static u64 _get_arg_as_int(runtime_global_state_t* global,runtime_local_state_t* local,const aml_object_t* object,u8 arg_index){
	if (arg_index>=object->arg_count||object->args[arg_index].type==AML_OBJECT_ARG_TYPE_STRING||object->args[arg_index].type==AML_OBJECT_ARG_TYPE_NAME){
		ERROR("Invalid argument");
		for (;;);
	}
	if (object->args[arg_index].type!=AML_OBJECT_ARG_TYPE_OBJECT){
		return object->args[arg_index].number;
	}
	aml_node_t* ret=_execute(global,local,object->args[arg_index].object);
	if (!ret||ret->type!=AML_NODE_TYPE_INTEGER){
		ERROR("Expected integer return value");
		for (;;);
	}
	return ret->data.integer;
}



static aml_node_t* _get_arg_as_node(runtime_global_state_t* global,runtime_local_state_t* local,const aml_object_t* object,u8 arg_index){
	if (arg_index>=object->arg_count){
		ERROR("Invalid argument");
		for (;;);
	}
	switch (object->args[arg_index].type){
		case AML_OBJECT_ARG_TYPE_UINT8:
		case AML_OBJECT_ARG_TYPE_UINT16:
		case AML_OBJECT_ARG_TYPE_UINT32:
		case AML_OBJECT_ARG_TYPE_UINT64:
			local->simple_return_value.type=AML_NODE_TYPE_INTEGER;
			local->simple_return_value.data.integer=object->args[arg_index].number;
			return &(local->simple_return_value);
		case AML_OBJECT_ARG_TYPE_STRING:
			ERROR("Unimplemented: AML_OBJECT_ARG_TYPE_STRING");for (;;);
			return NULL;
		case AML_OBJECT_ARG_TYPE_NAME:
			ERROR("Unimplemented: AML_OBJECT_ARG_TYPE_NAME");for (;;);
			return NULL;
		case AML_OBJECT_ARG_TYPE_OBJECT:
			return _execute(global,local,object->args[arg_index].object);
	}
	return NULL;
}



static aml_node_t* _execute(runtime_global_state_t* global,runtime_local_state_t* local,const aml_object_t* object){
	switch (MAKE_OPCODE(object->opcode[0],object->opcode[1])){
		case MAKE_OPCODE(AML_OPCODE_ZERO,0):
			local->simple_return_value.type=AML_NODE_TYPE_INTEGER;
			local->simple_return_value.data.integer=0;
			return &(local->simple_return_value);
		case MAKE_OPCODE(AML_OPCODE_ONE,0):
			local->simple_return_value.type=AML_NODE_TYPE_INTEGER;
			local->simple_return_value.data.integer=1;
			return &(local->simple_return_value);
		case MAKE_OPCODE(AML_OPCODE_ALIAS,0):
			ERROR("Unimplemented: AML_OPCODE_ALIAS");for (;;);
			return NULL;
		case MAKE_OPCODE(AML_OPCODE_NAME,0):
			{
				aml_node_t* value=_get_arg_as_node(global,local,object,1);
				if (!(value->flags&AML_NODE_FLAG_LOCAL)&&value->parent!=value){
					ERROR("Unimplemented: Name as reference?");
					for (;;);
				}
				aml_node_t* out=_get_node(global,local,object->args[0].string,value->type);
				out->data=value->data;
				return out;
			}
		case MAKE_OPCODE(AML_OPCODE_BYTE_PREFIX,0):
			local->simple_return_value.type=AML_NODE_TYPE_INTEGER;
			local->simple_return_value.data.integer=object->args[0].number;
			return &(local->simple_return_value);
		case MAKE_OPCODE(AML_OPCODE_WORD_PREFIX,0):
			local->simple_return_value.type=AML_NODE_TYPE_INTEGER;
			local->simple_return_value.data.integer=object->args[0].number;
			return &(local->simple_return_value);
		case MAKE_OPCODE(AML_OPCODE_DWORD_PREFIX,0):
			local->simple_return_value.type=AML_NODE_TYPE_INTEGER;
			local->simple_return_value.data.integer=object->args[0].number;
			return &(local->simple_return_value);
		case MAKE_OPCODE(AML_OPCODE_STRING_PREFIX,0):
			local->simple_return_value.type=AML_NODE_TYPE_STRING;
			local->simple_return_value.data.string.length=object->args[0].string_length;
			local->simple_return_value.data.string.data=object->args[0].string;
			return &(local->simple_return_value);
		case MAKE_OPCODE(AML_OPCODE_QWORD_PREFIX,0):
			local->simple_return_value.type=AML_NODE_TYPE_INTEGER;
			local->simple_return_value.data.integer=object->args[0].number;
			return &(local->simple_return_value);
		case MAKE_OPCODE(AML_OPCODE_SCOPE,0):
			{
				aml_node_t* prev_namespace=local->namespace;
				aml_node_t* scope=_get_node(global,local,object->args[0].string,AML_NODE_TYPE_SCOPE);
				local->namespace=scope;
				_execute_multiple(global,local,object->data.objects,object->data_length);
				local->namespace=prev_namespace;
				return scope;
			}
		case MAKE_OPCODE(AML_OPCODE_BUFFER,0):
			{
				u64 size=_get_arg_as_int(global,local,object,0);
				u8* buffer_data=kmm_allocate(size);
				memset(buffer_data,0,size);
				memcpy(buffer_data,object->data.bytes,(size<object->data_length?size:object->data_length));
				local->simple_return_value.type=AML_NODE_TYPE_BUFFER;
				local->simple_return_value.data.buffer.length=size;
				local->simple_return_value.data.buffer.data=buffer_data;
				return &(local->simple_return_value);
			}
		case MAKE_OPCODE(AML_OPCODE_PACKAGE,0):
		case MAKE_OPCODE(AML_OPCODE_VAR_PACKAGE,0):
			{
				aml_node_t* package=_alloc_node(NULL,AML_NODE_TYPE_PACKAGE,NULL);
				package->data.package.length=_get_arg_as_int(global,local,object,0);
				package->data.package.elements=kmm_allocate(package->data.package.length*sizeof(aml_node_t));
				for (u8 i=0;i<(object->data_length<package->data.package.length?object->data_length:package->data.package.length);i++){
					aml_node_t* value=_execute(global,local,object->data.objects+i);
					if (value->flags&AML_NODE_FLAG_LOCAL){
						*(package->data.package.elements+i)=*value;
					}
					else{
						ERROR("Unimplemented");
						for (;;);
					}
				}
				for (u8 i=object->data_length;i<package->data.package.length;i++){
					(package->data.package.elements+i)->type=AML_NODE_TYPE_UNDEFINED;
				}
				return package;
			}
		case MAKE_OPCODE(AML_OPCODE_METHOD,0):
			{
				aml_node_t* method=_get_node(global,local,object->args[0].string,AML_NODE_TYPE_METHOD);
				method->data.method.flags=object->args[1].number;
				method->data.method.objects=object->data.objects;
				method->data.method.object_count=object->data_length;
				return method;
			}
		case MAKE_OPCODE(AML_OPCODE_LOCAL0,0):
		case MAKE_OPCODE(AML_OPCODE_LOCAL1,0):
		case MAKE_OPCODE(AML_OPCODE_LOCAL2,0):
		case MAKE_OPCODE(AML_OPCODE_LOCAL3,0):
		case MAKE_OPCODE(AML_OPCODE_LOCAL4,0):
		case MAKE_OPCODE(AML_OPCODE_LOCAL5,0):
		case MAKE_OPCODE(AML_OPCODE_LOCAL6,0):
		case MAKE_OPCODE(AML_OPCODE_LOCAL7,0):
			ERROR("Unimplemented: AML_OPCODE_LOCAL%u",object->opcode[0]-AML_OPCODE_LOCAL0);for (;;);
			return NULL;
		case MAKE_OPCODE(AML_OPCODE_ARG0,0):
		case MAKE_OPCODE(AML_OPCODE_ARG1,0):
		case MAKE_OPCODE(AML_OPCODE_ARG2,0):
		case MAKE_OPCODE(AML_OPCODE_ARG3,0):
		case MAKE_OPCODE(AML_OPCODE_ARG4,0):
		case MAKE_OPCODE(AML_OPCODE_ARG5,0):
		case MAKE_OPCODE(AML_OPCODE_ARG6,0):
			ERROR("Unimplemented: AML_OPCODE_ARG%u",object->opcode[0]-AML_OPCODE_ARG0);for (;;);
			return NULL;
		case MAKE_OPCODE(AML_OPCODE_STORE,0):
			ERROR("Unimplemented: AML_OPCODE_STORE");for (;;);
			return NULL;
		case MAKE_OPCODE(AML_OPCODE_REF_OF,0):
			ERROR("Unimplemented: AML_OPCODE_REF_OF");for (;;);
			return NULL;
		case MAKE_OPCODE(AML_OPCODE_ADD,0):
			ERROR("Unimplemented: AML_OPCODE_ADD");for (;;);
			return NULL;
		case MAKE_OPCODE(AML_OPCODE_CONCAT,0):
			ERROR("Unimplemented: AML_OPCODE_CONCAT");for (;;);
			return NULL;
		case MAKE_OPCODE(AML_OPCODE_SUBTRACT,0):
			ERROR("Unimplemented: AML_OPCODE_SUBTRACT");for (;;);
			return NULL;
		case MAKE_OPCODE(AML_OPCODE_INCREMENT,0):
			ERROR("Unimplemented: AML_OPCODE_INCREMENT");for (;;);
			return NULL;
		case MAKE_OPCODE(AML_OPCODE_DECREMENT,0):
			ERROR("Unimplemented: AML_OPCODE_DECREMENT");for (;;);
			return NULL;
		case MAKE_OPCODE(AML_OPCODE_MULTIPLY,0):
			ERROR("Unimplemented: AML_OPCODE_MULTIPLY");for (;;);
			return NULL;
		case MAKE_OPCODE(AML_OPCODE_DIVIDE,0):
			ERROR("Unimplemented: AML_OPCODE_DIVIDE");for (;;);
			return NULL;
		case MAKE_OPCODE(AML_OPCODE_SHIFT_LEFT,0):
			ERROR("Unimplemented: AML_OPCODE_SHIFT_LEFT");for (;;);
			return NULL;
		case MAKE_OPCODE(AML_OPCODE_SHIFT_RIGHT,0):
			ERROR("Unimplemented: AML_OPCODE_SHIFT_RIGHT");for (;;);
			return NULL;
		case MAKE_OPCODE(AML_OPCODE_AND,0):
			ERROR("Unimplemented: AML_OPCODE_AND");for (;;);
			return NULL;
		case MAKE_OPCODE(AML_OPCODE_NAND,0):
			ERROR("Unimplemented: AML_OPCODE_NAND");for (;;);
			return NULL;
		case MAKE_OPCODE(AML_OPCODE_OR,0):
			ERROR("Unimplemented: AML_OPCODE_OR");for (;;);
			return NULL;
		case MAKE_OPCODE(AML_OPCODE_NOR,0):
			ERROR("Unimplemented: AML_OPCODE_NOR");for (;;);
			return NULL;
		case MAKE_OPCODE(AML_OPCODE_XOR,0):
			ERROR("Unimplemented: AML_OPCODE_XOR");for (;;);
			return NULL;
		case MAKE_OPCODE(AML_OPCODE_NOT,0):
			ERROR("Unimplemented: AML_OPCODE_NOT");for (;;);
			return NULL;
		case MAKE_OPCODE(AML_OPCODE_FIND_SET_LEFT_BIT,0):
			ERROR("Unimplemented: AML_OPCODE_FIND_SET_LEFT_BIT");for (;;);
			return NULL;
		case MAKE_OPCODE(AML_OPCODE_FIND_SET_RIGHT_BIT,0):
			ERROR("Unimplemented: AML_OPCODE_FIND_SET_RIGHT_BIT");for (;;);
			return NULL;
		case MAKE_OPCODE(AML_OPCODE_DEREF_OF,0):
			ERROR("Unimplemented: AML_OPCODE_DEREF_OF");for (;;);
			return NULL;
		case MAKE_OPCODE(AML_OPCODE_CONCAT_RES,0):
			ERROR("Unimplemented: AML_OPCODE_CONCAT_RES");for (;;);
			return NULL;
		case MAKE_OPCODE(AML_OPCODE_MOD,0):
			ERROR("Unimplemented: AML_OPCODE_MOD");for (;;);
			return NULL;
		case MAKE_OPCODE(AML_OPCODE_NOTIFY,0):
			ERROR("Unimplemented: AML_OPCODE_NOTIFY");for (;;);
			return NULL;
		case MAKE_OPCODE(AML_OPCODE_SIZE_OF,0):
			ERROR("Unimplemented: AML_OPCODE_SIZE_OF");for (;;);
			return NULL;
		case MAKE_OPCODE(AML_OPCODE_INDEX,0):
			ERROR("Unimplemented: AML_OPCODE_INDEX");for (;;);
			return NULL;
		case MAKE_OPCODE(AML_OPCODE_MATCH,0):
			ERROR("Unimplemented: AML_OPCODE_MATCH");for (;;);
			return NULL;
		case MAKE_OPCODE(AML_OPCODE_CREATE_DWORD_FIELD,0):
			ERROR("Unimplemented: AML_OPCODE_CREATE_DWORD_FIELD");for (;;);
			return NULL;
		case MAKE_OPCODE(AML_OPCODE_CREATE_WORD_FIELD,0):
			ERROR("Unimplemented: AML_OPCODE_CREATE_WORD_FIELD");for (;;);
			return NULL;
		case MAKE_OPCODE(AML_OPCODE_CREATE_BYTE_FIELD,0):
			ERROR("Unimplemented: AML_OPCODE_CREATE_BYTE_FIELD");for (;;);
			return NULL;
		case MAKE_OPCODE(AML_OPCODE_CREATE_BIT_FIELD,0):
			ERROR("Unimplemented: AML_OPCODE_CREATE_BIT_FIELD");for (;;);
			return NULL;
		case MAKE_OPCODE(AML_OPCODE_OBJECT_TYPE,0):
			ERROR("Unimplemented: AML_OPCODE_OBJECT_TYPE");for (;;);
			return NULL;
		case MAKE_OPCODE(AML_OPCODE_CREATE_QWORD_FIELD,0):
			ERROR("Unimplemented: AML_OPCODE_CREATE_QWORD_FIELD");for (;;);
			return NULL;
		case MAKE_OPCODE(AML_OPCODE_L_AND,0):
			ERROR("Unimplemented: AML_OPCODE_L_AND");for (;;);
			return NULL;
		case MAKE_OPCODE(AML_OPCODE_L_OR,0):
			ERROR("Unimplemented: AML_OPCODE_L_OR");for (;;);
			return NULL;
		case MAKE_OPCODE(AML_OPCODE_L_NOT,0):
			ERROR("Unimplemented: AML_OPCODE_L_NOT");for (;;);
			return NULL;
		case MAKE_OPCODE(AML_OPCODE_L_EQUAL,0):
			ERROR("Unimplemented: AML_OPCODE_L_EQUAL");for (;;);
			return NULL;
		case MAKE_OPCODE(AML_OPCODE_L_GREATER,0):
			ERROR("Unimplemented: AML_OPCODE_L_GREATER");for (;;);
			return NULL;
		case MAKE_OPCODE(AML_OPCODE_L_LESS,0):
			ERROR("Unimplemented: AML_OPCODE_L_LESS");for (;;);
			return NULL;
		case MAKE_OPCODE(AML_OPCODE_TO_BUFFER,0):
			ERROR("Unimplemented: AML_OPCODE_TO_BUFFER");for (;;);
			return NULL;
		case MAKE_OPCODE(AML_OPCODE_TO_DECIMAL_STRING,0):
			ERROR("Unimplemented: AML_OPCODE_TO_DECIMAL_STRING");for (;;);
			return NULL;
		case MAKE_OPCODE(AML_OPCODE_TO_HEX_STRING,0):
			ERROR("Unimplemented: AML_OPCODE_TO_HEX_STRING");for (;;);
			return NULL;
		case MAKE_OPCODE(AML_OPCODE_TO_INTEGER,0):
			ERROR("Unimplemented: AML_OPCODE_TO_INTEGER");for (;;);
			return NULL;
		case MAKE_OPCODE(AML_OPCODE_TO_STRING,0):
			ERROR("Unimplemented: AML_OPCODE_TO_STRING");for (;;);
			return NULL;
		case MAKE_OPCODE(AML_OPCODE_COPY_OBJECT,0):
			ERROR("Unimplemented: AML_OPCODE_COPY_OBJECT");for (;;);
			return NULL;
		case MAKE_OPCODE(AML_OPCODE_MID,0):
			ERROR("Unimplemented: AML_OPCODE_MID");for (;;);
			return NULL;
		case MAKE_OPCODE(AML_OPCODE_CONTINUE,0):
			ERROR("Unimplemented: AML_OPCODE_CONTINUE");for (;;);
			return NULL;
		case MAKE_OPCODE(AML_OPCODE_IF,0):
			ERROR("Unimplemented: AML_OPCODE_IF");for (;;);
			return NULL;
		case MAKE_OPCODE(AML_OPCODE_ELSE,0):
			ERROR("Unimplemented: AML_OPCODE_ELSE");for (;;);
			return NULL;
		case MAKE_OPCODE(AML_OPCODE_WHILE,0):
			ERROR("Unimplemented: AML_OPCODE_WHILE");for (;;);
			return NULL;
		case MAKE_OPCODE(AML_OPCODE_NOOP,0):
			ERROR("Unimplemented: AML_OPCODE_NOOP");for (;;);
			return NULL;
		case MAKE_OPCODE(AML_OPCODE_RETURN,0):
			ERROR("Unimplemented: AML_OPCODE_RETURN");for (;;);
			return NULL;
		case MAKE_OPCODE(AML_OPCODE_BREAK,0):
			ERROR("Unimplemented: AML_OPCODE_BREAK");for (;;);
			return NULL;
		case MAKE_OPCODE(AML_OPCODE_BREAK_POINT,0):
			ERROR("Unimplemented: AML_OPCODE_BREAK_POINT");for (;;);
			return NULL;
		case MAKE_OPCODE(AML_OPCODE_ONES,0):
			local->simple_return_value.type=AML_NODE_TYPE_INTEGER;
			local->simple_return_value.data.integer=0xffffffffffffffffull;
			return &(local->simple_return_value);
		case MAKE_OPCODE(AML_EXTENDED_OPCODE,AML_OPCODE_EXT_MUTEX):
			{
				aml_node_t* mutex=_get_node(global,local,object->args[0].string,AML_NODE_TYPE_MUTEX);
				return mutex;
			}
		case MAKE_OPCODE(AML_EXTENDED_OPCODE,AML_OPCODE_EXT_EVENT):
			ERROR("Unimplemented: AML_OPCODE_EXT_EVENT");for (;;);
			return NULL;
		case MAKE_OPCODE(AML_EXTENDED_OPCODE,AML_OPCODE_EXT_COND_REF_OF):
			ERROR("Unimplemented: AML_OPCODE_EXT_COND_REF_OF");for (;;);
			return NULL;
		case MAKE_OPCODE(AML_EXTENDED_OPCODE,AML_OPCODE_EXT_CREATE_FIELD):
			ERROR("Unimplemented: AML_OPCODE_EXT_CREATE_FIELD");for (;;);
			return NULL;
		case MAKE_OPCODE(AML_EXTENDED_OPCODE,AML_OPCODE_EXT_LOAD_TABLE):
			ERROR("Unimplemented: AML_OPCODE_EXT_LOAD_TABLE");for (;;);
			return NULL;
		case MAKE_OPCODE(AML_EXTENDED_OPCODE,AML_OPCODE_EXT_LOAD):
			ERROR("Unimplemented: AML_OPCODE_EXT_LOAD");for (;;);
			return NULL;
		case MAKE_OPCODE(AML_EXTENDED_OPCODE,AML_OPCODE_EXT_STALL):
			ERROR("Unimplemented: AML_OPCODE_EXT_STALL");for (;;);
			return NULL;
		case MAKE_OPCODE(AML_EXTENDED_OPCODE,AML_OPCODE_EXT_SLEEP):
			ERROR("Unimplemented: AML_OPCODE_EXT_SLEEP");for (;;);
			return NULL;
		case MAKE_OPCODE(AML_EXTENDED_OPCODE,AML_OPCODE_EXT_ACQUIRE):
			ERROR("Unimplemented: AML_OPCODE_EXT_ACQUIRE");for (;;);
			return NULL;
		case MAKE_OPCODE(AML_EXTENDED_OPCODE,AML_OPCODE_EXT_SIGNAL):
			ERROR("Unimplemented: AML_OPCODE_EXT_SIGNAL");for (;;);
			return NULL;
		case MAKE_OPCODE(AML_EXTENDED_OPCODE,AML_OPCODE_EXT_WAIT):
			ERROR("Unimplemented: AML_OPCODE_EXT_WAIT");for (;;);
			return NULL;
		case MAKE_OPCODE(AML_EXTENDED_OPCODE,AML_OPCODE_EXT_RESET):
			ERROR("Unimplemented: AML_OPCODE_EXT_RESET");for (;;);
			return NULL;
		case MAKE_OPCODE(AML_EXTENDED_OPCODE,AML_OPCODE_EXT_RELEASE):
			ERROR("Unimplemented: AML_OPCODE_EXT_RELEASE");for (;;);
			return NULL;
		case MAKE_OPCODE(AML_EXTENDED_OPCODE,AML_OPCODE_EXT_FROM_BCD):
			ERROR("Unimplemented: AML_OPCODE_EXT_FROM_BCD");for (;;);
			return NULL;
		case MAKE_OPCODE(AML_EXTENDED_OPCODE,AML_OPCODE_EXT_TO_BCD):
			ERROR("Unimplemented: AML_OPCODE_EXT_TO_BCD");for (;;);
			return NULL;
		case MAKE_OPCODE(AML_EXTENDED_OPCODE,AML_OPCODE_EXT_UNLOAD):
			ERROR("Unimplemented: AML_OPCODE_EXT_UNLOAD");for (;;);
			return NULL;
		case MAKE_OPCODE(AML_EXTENDED_OPCODE,AML_OPCODE_EXT_REVISION):
			ERROR("Unimplemented: AML_OPCODE_EXT_REVISION");for (;;);
			return NULL;
		case MAKE_OPCODE(AML_EXTENDED_OPCODE,AML_OPCODE_EXT_DEBUG):
			ERROR("Unimplemented: AML_OPCODE_EXT_DEBUG");for (;;);
			return NULL;
		case MAKE_OPCODE(AML_EXTENDED_OPCODE,AML_OPCODE_EXT_FATAL):
			ERROR("Unimplemented: AML_OPCODE_EXT_FATAL");for (;;);
			return NULL;
		case MAKE_OPCODE(AML_EXTENDED_OPCODE,AML_OPCODE_EXT_TIMER):
			ERROR("Unimplemented: AML_OPCODE_EXT_TIMER");for (;;);
			return NULL;
		case MAKE_OPCODE(AML_EXTENDED_OPCODE,AML_OPCODE_EXT_REGION):
			{
				aml_node_t* region=_get_node(global,local,object->args[0].string,AML_NODE_TYPE_REGION);
				region->data.region.type=object->args[1].number;
				region->data.region.offset=_get_arg_as_int(global,local,object,2);
				region->data.region.length=_get_arg_as_int(global,local,object,3);
				return region;
			}
		case MAKE_OPCODE(AML_EXTENDED_OPCODE,AML_OPCODE_EXT_FIELD):
			{
				aml_node_t* region=_get_node(global,local,object->args[0].string,AML_NODE_TYPE_REGION);
				u8 region_type=region->data.region.type;
				u64 region_start=region->data.region.offset;
				u64 region_end=region_start+region->data.region.length;
				u64 offset=region_start;
				for (u32 i=0;i<object->data_length;){
					if (!object->data.bytes[i]){
						i++;
						u32 size;
						i+=aml_parse_pkglength(object->data.bytes+i,&size);
						offset+=size;
						continue;
					}
					if (object->data.bytes[i]<32){
						ERROR("Unimplemented special field type: %x",object->data.bytes[i]);
						for (;;);
					}
					aml_node_t* field_unit=_alloc_node((const char*)(object->data.bytes+i),AML_NODE_TYPE_FIELD_UNIT,region);
					i+=4;
					u32 size;
					i+=aml_parse_pkglength(object->data.bytes+i,&size);
					field_unit->data.field_unit.type=region_type;
					field_unit->data.field_unit.access_type=object->args[1].number;
					field_unit->data.field_unit.address=region_start+offset;
					field_unit->data.field_unit.size=size;
					offset+=size;
				}
				if (offset-region_start>region_end){
					WARN("Region fields do not cover the entire field");
				}
				return region;
			}
		case MAKE_OPCODE(AML_EXTENDED_OPCODE,AML_OPCODE_EXT_DEVICE):
			{
				aml_node_t* prev_namespace=local->namespace;
				aml_node_t* device=_get_node(global,local,object->args[0].string,AML_NODE_TYPE_DEVICE);
				local->namespace=device;
				_execute_multiple(global,local,object->data.objects,object->data_length);
				local->namespace=prev_namespace;
				return device;
			}
		case MAKE_OPCODE(AML_EXTENDED_OPCODE,AML_OPCODE_EXT_PROCESSOR):
			{
				aml_node_t* prev_namespace=local->namespace;
				aml_node_t* processor=_get_node(global,local,object->args[0].string,AML_NODE_TYPE_PROCESSOR);
				processor->data.processor.id=object->args[1].number;
				processor->data.processor.block_address=object->args[2].number;
				processor->data.processor.block_length=object->args[3].number;
				local->namespace=processor;
				_execute_multiple(global,local,object->data.objects,object->data_length);
				local->namespace=prev_namespace;
				return processor;
			}
		case MAKE_OPCODE(AML_EXTENDED_OPCODE,AML_OPCODE_EXT_POWER_RES):
			ERROR("Unimplemented: AML_OPCODE_EXT_POWER_RES");for (;;);
			return NULL;
		case MAKE_OPCODE(AML_EXTENDED_OPCODE,AML_OPCODE_EXT_THERMAL_ZONE):
			ERROR("Unimplemented: AML_OPCODE_EXT_THERMAL_ZONE");for (;;);
			return NULL;
		case MAKE_OPCODE(AML_EXTENDED_OPCODE,AML_OPCODE_EXT_INDEX_FIELD):
			ERROR("Unimplemented: AML_OPCODE_EXT_INDEX_FIELD");for (;;);
			return NULL;
		case MAKE_OPCODE(AML_EXTENDED_OPCODE,AML_OPCODE_EXT_BANK_FIELD):
			ERROR("Unimplemented: AML_OPCODE_EXT_BANK_FIELD");for (;;);
			return NULL;
		case MAKE_OPCODE(AML_EXTENDED_OPCODE,AML_OPCODE_EXT_DATA_REGION):
			ERROR("Unimplemented: AML_OPCODE_EXT_DATA_REGION");for (;;);
			return NULL;
		case MAKE_OPCODE(AML_OPCODE_STRING,0):
			ERROR("Unimplemented: AML_OPCODE_STRING");for (;;);
			return NULL;
	}
	return NULL;
}



static void _execute_multiple(runtime_global_state_t* global,runtime_local_state_t* local,const aml_object_t* objects,u32 object_count){
	for (u32 i=0;i<object_count;i++){
		_execute(global,local,objects+i);
	}
}



static void _print_node(const aml_node_t* node,u32 indent,_Bool inside_package){
	for (u32 i=0;i<indent;i+=4){
		log("    ");
	}
	if (!inside_package){
		log("%s:",node->name);
	}
	switch (node->type){
		case AML_NODE_TYPE_UNDEFINED:
			log("<undefined>");
			return;
		case AML_NODE_TYPE_BUFFER:
			log("buffer<size=%u>",node->data.buffer.length);
			if (node->data.buffer.length>16){
				return;
			}
			log("{",node->data.buffer.length);
			for (u8 i=0;i<node->data.buffer.length;i++){
				if (i){
					log(" ");
				}
				log("%x",node->data.buffer.data[i]);
			}
			log("}");
			return;
		case AML_NODE_TYPE_BUFFER_FIELD:
			log("buffer_field<...>");
			break;
		case AML_NODE_TYPE_DEBUG:
			log("debug<...>");
			break;
		case AML_NODE_TYPE_DEVICE:
			log("device");
			break;
		case AML_NODE_TYPE_EVENT:
			log("event<...>");
			return;
		case AML_NODE_TYPE_FIELD_UNIT:
			log("field_unit<address=%p, size=%u>",node->data.field_unit.address,node->data.field_unit.size);
			return;
		case AML_NODE_TYPE_INTEGER:
			log("0x%lx",node->data.integer);
			return;
		case AML_NODE_TYPE_METHOD:
			log("method<arg_count=%u>",node->data.method.flags&7);
			return;
		case AML_NODE_TYPE_MUTEX:
			log("mutex<...>");
			return;
		case AML_NODE_TYPE_REFERENCE:
			log("reference<...>");
			return;
		case AML_NODE_TYPE_REGION:
			log("region<type=");
			switch (node->data.region.type){
				case 0:
					log("SystemMemory");
					break;
				case 1:
					log("SystemIO");
					break;
				case 2:
					log("PCI_Config");
					break;
				case 3:
					log("EmbeddedControl");
					break;
				case 4:
					log("SMBus");
					break;
				case 5:
					log("SystemCMOS");
					break;
				case 6:
					log("PciBarTarget");
					break;
				case 7:
					log("IPMI");
					break;
				case 8:
					log("GeneralPurposeIO");
					break;
				case 9:
					log("GenericSerialBus");
					break;
				case 10:
					log("PCC");
					break;
				default:
					log("Unknown");
					break;
			}
			log(", offset=%u, length=%u>",node->data.region.offset,node->data.region.length);
			break;
		case AML_NODE_TYPE_POWER_RESOURCE:
			log("power_resource<...>");
			break;
		case AML_NODE_TYPE_PROCESSOR:
			log("processor<id=%u>",node->data.processor.id);
			break;
		case AML_NODE_TYPE_STRING:
			log("'%s'",node->data.string.data);
			return;
		case AML_NODE_TYPE_THERMAL_ZONE:
			log("thermal_zone<...>");
			break;
	}
	if ((node->type==AML_NODE_TYPE_PACKAGE?!node->data.package.length:!node->child)){
		log("{}");
		return;
	}
	log("{\n");
	if (node->type==AML_NODE_TYPE_PACKAGE){
		for (u8 i=0;i<node->data.package.length;i++){
			_print_node(node->data.package.elements+i,indent+4,1);
			if (i+1<node->data.package.length){
				log(",\n");
			}
			else{
				log("\n");
			}
		}
	}
	else{
		for (aml_node_t* child=node->child;child;child=child->next){
			_print_node(child,indent+4,0);
			if (child->next){
				log(",\n");
			}
			else{
				log("\n");
			}
		}
	}
	for (u32 i=0;i<indent;i+=4){
		log("    ");
	}
	log("}");
	if (!indent){
		log("\n");
	}
}



void aml_build_runtime(aml_object_t* root){
	LOG("Building AML runtime...");
	aml_root_node=_alloc_node("\\\x00\x00\x00",AML_NODE_TYPE_SCOPE,NULL);
	runtime_global_state_t global={
		aml_root_node
	};
	runtime_local_state_t local={
		global.root_namespace
	};
	local.simple_return_value.flags=AML_NODE_FLAG_LOCAL;
	_execute_multiple(&global,&local,root->data.objects,root->data_length);
	(void)_print_node;//_print_node(global.root_namespace,0,0);
}
