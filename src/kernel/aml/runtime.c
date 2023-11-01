#include <kernel/aml/parser.h>
#include <kernel/aml/runtime.h>
#include <kernel/apic/ioapic.h>
#include <kernel/io/io.h>
#include <kernel/isr/isr.h>
#include <kernel/lock/spinlock.h>
#include <kernel/log/log.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#include <kernel/memory/smm.h>
#define KERNEL_LOG_NAME "aml"



#define MAKE_OPCODE(a,b) ((a)|((b)<<8))



typedef struct _RUNTIME_LOCAL_STATE{
	aml_node_t* namespace;
	aml_node_t* args[7];
	aml_node_t* locals[8];
	aml_node_t arg_simple_values[7];
	aml_node_t local_simple_values[8];
	aml_node_t simple_return_value;
	aml_node_t* return_value;
	_Bool was_branch_taken;
} runtime_local_state_t;



static pmm_counter_descriptor_t _aml_node_omm_pmm_counter=PMM_COUNTER_INIT_STRUCT("omm_aml_node");
static omm_allocator_t _aml_node_allocator=OMM_ALLOCATOR_INIT_STRUCT("aml_node",sizeof(aml_node_t),8,4,&_aml_node_omm_pmm_counter);



aml_node_t* aml_root_node;



static aml_node_t* _execute(runtime_local_state_t* local,const aml_object_t* object);



static void _execute_multiple(runtime_local_state_t* local,const aml_object_t* first_object,u32 object_count);



static u64 _read_field_unit(aml_node_t* node){
	u64 out=0;
	switch (node->data.field_unit.type){
		case 0x00:
			if (node->data.field_unit.access_type&16){
				spinlock_acquire_exclusive(&(node->data.field_unit.lock));
			}
			switch (node->data.field_unit.access_type&15){
				case 0:
					panic("Unimplemented: _read_field_unit(SystemMemory,AnyAcc)");
					break;
				case 1:
					panic("Unimplemented: _read_field_unit(SystemMemory,ByteAcc)");
					break;
				case 2:
					panic("Unimplemented: _read_field_unit(SystemMemory,WordAcc)");
					break;
				case 3:
					out=*((const u32*)(node->data.field_unit.address+VMM_HIGHER_HALF_ADDRESS_OFFSET));
					break;
				case 4:
					panic("Unimplemented: _read_field_unit(SystemMemory,QWordAcc)");
					break;
				case 5:
					panic("Unimplemented: _read_field_unit(SystemMemory,BufferAcc)");
					break;
			}
			if (node->data.field_unit.access_type&16){
				spinlock_release_exclusive(&(node->data.field_unit.lock));
			}
			break;
		case 0x01:
			panic("Unimplemented: _read_field_unit(SystemIO)");
			break;
		case 0x02:
			panic("Unimplemented: _read_field_unit(PCI_Config)");
			break;
		case 0x03:
			panic("Unimplemented: _read_field_unit(EmbeddedControl)");
			break;
		case 0x04:
			panic("Unimplemented: _read_field_unit(SMBus)");
			break;
		case 0x05:
			panic("Unimplemented: _read_field_unit(System CMOS)");
			break;
		case 0x06:
			panic("Unimplemented: _read_field_unit(PciBarTarget)");
			break;
		case 0x07:
			panic("Unimplemented: _read_field_unit(IPMI)");
			break;
		case 0x08:
			panic("Unimplemented: _read_field_unit(GeneralPurposeIO)");
			break;
		case 0x09:
			panic("Unimplemented: _read_field_unit(GenericSerialBus)");
			break;
		case 0x0a:
			panic("Unimplemented: _read_field_unit(PCC)");
			break;
	}
	return out;
}



static void _write_field_unit(aml_node_t* node,aml_node_t* value){
	if (node->data.field_unit.access_type&16){
		spinlock_acquire_exclusive(&(node->data.field_unit.lock));
	}
	if (value->type!=AML_NODE_TYPE_INTEGER){
		ERROR("Unable to convert %u to %u",value->type,AML_NODE_TYPE_INTEGER);for (;;);
	}
	switch (node->data.field_unit.type){
		case 0x00:
			switch (node->data.field_unit.access_type&15){
				case 0:
					panic("Unimplemented: _write_field_unit(SystemMemory,AnyAcc)");
					break;
				case 1:
					panic("Unimplemented: _write_field_unit(SystemMemory,ByteAcc)");
					break;
				case 2:
					panic("Unimplemented: _write_field_unit(SystemMemory,WordAcc)");
					break;
				case 3:
					*((u32*)(node->data.field_unit.address+VMM_HIGHER_HALF_ADDRESS_OFFSET))=value->data.integer;
					break;
				case 4:
					panic("Unimplemented: _write_field_unit(SystemMemory,QWordAcc)");
					break;
				case 5:
					panic("Unimplemented: _write_field_unit(SystemMemory,BufferAcc)");
					break;
			}
			break;
		case 0x01:
			switch (node->data.field_unit.access_type&15){
				case 0:
					panic("Unimplemented: _write_field_unit(SystemIO,AnyAcc)");
					break;
				case 1:
					panic("Unimplemented: _write_field_unit(SystemIO,ByteAcc)");
					break;
				case 2:
					panic("Unimplemented: _write_field_unit(SystemIO,WordAcc)");
					break;
				case 3:
					io_port_out32(node->data.field_unit.address,value->data.integer);
					break;
				case 4:
					panic("Unimplemented: _write_field_unit(SystemIO,QWordAcc)");
					break;
				case 5:
					panic("Unimplemented: _write_field_unit(SystemIO,BufferAcc)");
					break;
			}
			break;
		case 0x02:
			panic("Unimplemented: _write_field_unit(PCI_Config)");
			break;
		case 0x03:
			panic("Unimplemented: _write_field_unit(EmbeddedControl)");
			break;
		case 0x04:
			panic("Unimplemented: _write_field_unit(SMBus)");
			break;
		case 0x05:
			panic("Unimplemented: _write_field_unit(System CMOS)");
			break;
		case 0x06:
			panic("Unimplemented: _write_field_unit(PciBarTarget)");
			break;
		case 0x07:
			panic("Unimplemented: _write_field_unit(IPMI)");
			break;
		case 0x08:
			panic("Unimplemented: _write_field_unit(GeneralPurposeIO)");
			break;
		case 0x09:
			panic("Unimplemented: _write_field_unit(GenericSerialBus)");
			break;
		case 0x0a:
			panic("Unimplemented: _write_field_unit(PCC)");
			break;
	}
	if (node->data.field_unit.access_type&16){
		spinlock_release_exclusive(&(node->data.field_unit.lock));
	}
}



static aml_node_t* _alloc_node(const char* name,u8 type,aml_node_t* parent){
	aml_node_t* out=omm_alloc(&_aml_node_allocator);
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



static aml_node_t* _get_node(aml_node_t* local_namespace,const string_t* namespace,u8 type,_Bool create_if_not_found){
	aml_node_t* out=local_namespace;
	u16 i=0;
	if (namespace->data[i]=='\\'){
		out=aml_root_node;
		i++;
	}
	while (i<namespace->length){
		if (namespace->data[i]=='^'){
			out=out->parent;
			i++;
		}
		else if (namespace->data[i]=='.'){
			i++;
		}
		else if (namespace->data[i]=='['){
			u64 index=0;
			for (i++;namespace->data[i]!=']';i++){
				if (namespace->data[i]<48||namespace->data[i]>57){
					WARN("Invalid index character: %c",namespace->data[i]);
					continue;
				}
				index=index*10+namespace->data[i]-48;
			}
			i++;
			if (out->type!=AML_NODE_TYPE_PACKAGE||index>=out->data.package.length){
				if (create_if_not_found){
					panic("Unable to create package entry");
				}
				return NULL;
			}
			for (out=out->data.package.child;index;out=out->next){
				index--;
			}
		}
		else{
			for (aml_node_t* child=out->child;child;child=child->next){
				if (*((const u32*)(child->name))==*((const u32*)(namespace->data+i))){
					out=child;
					goto _namespace_found;
				}
			}
			if (!create_if_not_found){
				return NULL;
			}
			out=_alloc_node(namespace->data+i,(i+4<namespace->length?AML_NODE_TYPE_SCOPE:type),out);
_namespace_found:
			i+=4;
		}
	}
	return out;
}



static u64 _get_arg_as_int(runtime_local_state_t* local,const aml_object_t* object,u8 arg_index){
	if (arg_index>=object->arg_count||object->args[arg_index].type==AML_OBJECT_ARG_TYPE_STRING||object->args[arg_index].type==AML_OBJECT_ARG_TYPE_NAME){
		panic("Invalid argument");
	}
	if (object->args[arg_index].type!=AML_OBJECT_ARG_TYPE_OBJECT){
		return object->args[arg_index].number;
	}
	aml_node_t* ret=_execute(local,object->args[arg_index].object);
	if (!ret||(ret->type!=AML_NODE_TYPE_FIELD_UNIT&&ret->type!=AML_NODE_TYPE_INTEGER)){
		panic("Expected integer return value");
	}
	if (ret->type==AML_NODE_TYPE_FIELD_UNIT){
		return _read_field_unit(ret);
	}
	return ret->data.integer;
}



static aml_node_t* _get_arg_as_node(runtime_local_state_t* local,const aml_object_t* object,u8 arg_index){
	if (arg_index>=object->arg_count){
		panic("Invalid argument");
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
			panic("Unimplemented: AML_OBJECT_ARG_TYPE_STRING");
			return NULL;
		case AML_OBJECT_ARG_TYPE_NAME:
			panic("Unimplemented: AML_OBJECT_ARG_TYPE_NAME");
			return NULL;
		case AML_OBJECT_ARG_TYPE_OBJECT:
			return _execute(local,object->args[arg_index].object);
	}
	return NULL;
}



static aml_node_t* _execute(runtime_local_state_t* local,const aml_object_t* object){
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
			panic("Unimplemented: AML_OPCODE_ALIAS");
			return NULL;
		case MAKE_OPCODE(AML_OPCODE_NAME,0):
			{
				aml_node_t* value=_get_arg_as_node(local,object,1);
				if (!(value->flags&AML_NODE_FLAG_LOCAL)&&value->parent!=value){
					panic("Unimplemented: Name as reference?");
				}
				aml_node_t* out=_get_node(local->namespace,object->args[0].string,value->type,1);
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
		case MAKE_OPCODE(AML_OPCODE_NAME_REFERENCE_PREFIX,0):
			local->simple_return_value.type=AML_NODE_TYPE_STRING;
			local->simple_return_value.data.string=smm_duplicate(object->args[0].string);
			return &(local->simple_return_value);
		case MAKE_OPCODE(AML_OPCODE_QWORD_PREFIX,0):
			local->simple_return_value.type=AML_NODE_TYPE_INTEGER;
			local->simple_return_value.data.integer=object->args[0].number;
			return &(local->simple_return_value);
		case MAKE_OPCODE(AML_OPCODE_SCOPE,0):
			{
				aml_node_t* scope=_get_node(local->namespace,object->args[0].string,AML_NODE_TYPE_SCOPE,1);
				aml_node_t* prev_namespace=local->namespace;
				local->namespace=scope;
				_execute_multiple(local,object->data.child,object->data_length);
				local->namespace=prev_namespace;
				return scope;
			}
		case MAKE_OPCODE(AML_OPCODE_BUFFER,0):
			{
				u64 size=_get_arg_as_int(local,object,0);
				local->simple_return_value.type=AML_NODE_TYPE_BUFFER;
				local->simple_return_value.data.buffer=smm_alloc(NULL,size);
				memcpy(local->simple_return_value.data.buffer->data,object->data.bytes,(size<object->data_length?size:object->data_length));
				smm_rehash(local->simple_return_value.data.buffer);
				return &(local->simple_return_value);
			}
		case MAKE_OPCODE(AML_OPCODE_PACKAGE,0):
		case MAKE_OPCODE(AML_OPCODE_VAR_PACKAGE,0):
			{
				aml_node_t* package=_alloc_node(NULL,AML_NODE_TYPE_PACKAGE,NULL);
				package->data.package.length=_get_arg_as_int(local,object,0);
				package->data.package.child=NULL;
				aml_node_t* last_child=NULL;
				for (aml_object_t* child=object->data.child;child;child=child->next){
					aml_node_t* value=_execute(local,child);
					if (value->flags&AML_NODE_FLAG_LOCAL){
						aml_node_t* new_node=omm_alloc(&_aml_node_allocator);
						*new_node=*value;
						value=new_node;
					}
					if (last_child){
						last_child->next=value;
					}
					else{
						package->data.package.child=value;
					}
					last_child=value;
				}
				if (object->data_length!=package->data.package.length){
					panic("Unimplemented");
				}
				return package;
			}
		case MAKE_OPCODE(AML_OPCODE_METHOD,0):
			{
				aml_node_t* method=_get_node(local->namespace,object->args[0].string,AML_NODE_TYPE_METHOD,1);
				method->data.method.flags=object->args[1].number;
				method->data.method.child_count=object->data_length;
				method->data.method.child=object->data.child;
				method->data.method.namespace=local->namespace;
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
			return local->locals[object->opcode[0]-AML_OPCODE_LOCAL0];
		case MAKE_OPCODE(AML_OPCODE_ARG0,0):
		case MAKE_OPCODE(AML_OPCODE_ARG1,0):
		case MAKE_OPCODE(AML_OPCODE_ARG2,0):
		case MAKE_OPCODE(AML_OPCODE_ARG3,0):
		case MAKE_OPCODE(AML_OPCODE_ARG4,0):
		case MAKE_OPCODE(AML_OPCODE_ARG5,0):
		case MAKE_OPCODE(AML_OPCODE_ARG6,0):
			ERROR("Unimplemented: AML_OPCODE_ARG%u",object->opcode[0]-AML_OPCODE_ARG0);
			panic("Unimplemented");
			return NULL;
		case MAKE_OPCODE(AML_OPCODE_STORE,0):
			{
				aml_node_t* value=_get_arg_as_node(local,object,0);
				if (object->args[1].type==AML_OBJECT_ARG_TYPE_NAME){
					panic("Unimplemented: AML_OPCODE_STORE / AML_OBJECT_ARG_TYPE_NAME");
					return NULL;
				}
				if (object->args[1].type!=AML_OBJECT_ARG_TYPE_OBJECT){
					return NULL;
				}
				aml_object_t* target=object->args[1].object;
				switch (MAKE_OPCODE(target->opcode[0],target->opcode[1])){
					case MAKE_OPCODE(AML_OPCODE_LOCAL0,0):
					case MAKE_OPCODE(AML_OPCODE_LOCAL1,0):
					case MAKE_OPCODE(AML_OPCODE_LOCAL2,0):
					case MAKE_OPCODE(AML_OPCODE_LOCAL3,0):
					case MAKE_OPCODE(AML_OPCODE_LOCAL4,0):
					case MAKE_OPCODE(AML_OPCODE_LOCAL5,0):
					case MAKE_OPCODE(AML_OPCODE_LOCAL6,0):
					case MAKE_OPCODE(AML_OPCODE_LOCAL7,0):
						if (value->flags&AML_NODE_FLAG_LOCAL){
							local->local_simple_values[target->opcode[0]-AML_OPCODE_LOCAL0]=*value;
							value=local->local_simple_values+target->opcode[0]-AML_OPCODE_LOCAL0;
							value->flags&=~AML_NODE_FLAG_LOCAL;
						}
						local->locals[target->opcode[0]-AML_OPCODE_LOCAL0]=value;
						return NULL;
					case MAKE_OPCODE(AML_OPCODE_NAME_REFERENCE,0):
						for (aml_node_t* namespace=local->namespace;1;namespace=namespace->parent){
							aml_node_t* out=_get_node(namespace,target->args[0].string,0,0);
							if (out){
								if (out->type==AML_NODE_TYPE_FIELD_UNIT){
									_write_field_unit(out,value);
								}
								else{
									ERROR("Unimplemented: AML_OPCODE_STORE / %x",out->type);
									panic("Unimplemented");
								}
								return NULL;
							}
							if (namespace==namespace->parent){
								break;
							}
						}
						ERROR("Object not found: %s",target->args[0].string);for (;;);
						return NULL;
				}
				ERROR("Unimplemented: AML_OPCODE_STORE / AML_OBJECT_ARG_TYPE_OBJECT (%x)",target->opcode[0]);for (;;);
				panic("Unimplemented");
				return NULL;
			}
		case MAKE_OPCODE(AML_OPCODE_REF_OF,0):
			panic("Unimplemented: AML_OPCODE_REF_OF");
			return NULL;
		case MAKE_OPCODE(AML_OPCODE_ADD,0):
			panic("Unimplemented: AML_OPCODE_ADD");
			return NULL;
		case MAKE_OPCODE(AML_OPCODE_CONCAT,0):
			panic("Unimplemented: AML_OPCODE_CONCAT");
			return NULL;
		case MAKE_OPCODE(AML_OPCODE_SUBTRACT,0):
			panic("Unimplemented: AML_OPCODE_SUBTRACT");
			return NULL;
		case MAKE_OPCODE(AML_OPCODE_INCREMENT,0):
			panic("Unimplemented: AML_OPCODE_INCREMENT");
			return NULL;
		case MAKE_OPCODE(AML_OPCODE_DECREMENT,0):
			panic("Unimplemented: AML_OPCODE_DECREMENT");
			return NULL;
		case MAKE_OPCODE(AML_OPCODE_MULTIPLY,0):
			panic("Unimplemented: AML_OPCODE_MULTIPLY");
			return NULL;
		case MAKE_OPCODE(AML_OPCODE_DIVIDE,0):
			panic("Unimplemented: AML_OPCODE_DIVIDE");
			return NULL;
		case MAKE_OPCODE(AML_OPCODE_SHIFT_LEFT,0):
			panic("Unimplemented: AML_OPCODE_SHIFT_LEFT");
			return NULL;
		case MAKE_OPCODE(AML_OPCODE_SHIFT_RIGHT,0):
			local->simple_return_value.type=AML_NODE_TYPE_INTEGER;
			local->simple_return_value.data.integer=_get_arg_as_int(local,object,0)>>_get_arg_as_int(local,object,1);
			return &(local->simple_return_value);
		case MAKE_OPCODE(AML_OPCODE_AND,0):
			panic("Unimplemented: AML_OPCODE_AND");
			return NULL;
		case MAKE_OPCODE(AML_OPCODE_NAND,0):
			panic("Unimplemented: AML_OPCODE_NAND");
			return NULL;
		case MAKE_OPCODE(AML_OPCODE_OR,0):
			panic("Unimplemented: AML_OPCODE_OR");
			return NULL;
		case MAKE_OPCODE(AML_OPCODE_NOR,0):
			panic("Unimplemented: AML_OPCODE_NOR");
			return NULL;
		case MAKE_OPCODE(AML_OPCODE_XOR,0):
			panic("Unimplemented: AML_OPCODE_XOR");
			return NULL;
		case MAKE_OPCODE(AML_OPCODE_NOT,0):
			panic("Unimplemented: AML_OPCODE_NOT");
			return NULL;
		case MAKE_OPCODE(AML_OPCODE_FIND_SET_LEFT_BIT,0):
			panic("Unimplemented: AML_OPCODE_FIND_SET_LEFT_BIT");
			return NULL;
		case MAKE_OPCODE(AML_OPCODE_FIND_SET_RIGHT_BIT,0):
			panic("Unimplemented: AML_OPCODE_FIND_SET_RIGHT_BIT");
			return NULL;
		case MAKE_OPCODE(AML_OPCODE_DEREF_OF,0):
			panic("Unimplemented: AML_OPCODE_DEREF_OF");
			return NULL;
		case MAKE_OPCODE(AML_OPCODE_CONCAT_RES,0):
			panic("Unimplemented: AML_OPCODE_CONCAT_RES");
			return NULL;
		case MAKE_OPCODE(AML_OPCODE_MOD,0):
			panic("Unimplemented: AML_OPCODE_MOD");
			return NULL;
		case MAKE_OPCODE(AML_OPCODE_NOTIFY,0):
			panic("Unimplemented: AML_OPCODE_NOTIFY");
			return NULL;
		case MAKE_OPCODE(AML_OPCODE_SIZE_OF,0):
			panic("Unimplemented: AML_OPCODE_SIZE_OF");
			return NULL;
		case MAKE_OPCODE(AML_OPCODE_INDEX,0):
			panic("Unimplemented: AML_OPCODE_INDEX");
			return NULL;
		case MAKE_OPCODE(AML_OPCODE_MATCH,0):
			panic("Unimplemented: AML_OPCODE_MATCH");
			return NULL;
		case MAKE_OPCODE(AML_OPCODE_CREATE_DWORD_FIELD,0):
			panic("Unimplemented: AML_OPCODE_CREATE_DWORD_FIELD");
			return NULL;
		case MAKE_OPCODE(AML_OPCODE_CREATE_WORD_FIELD,0):
			panic("Unimplemented: AML_OPCODE_CREATE_WORD_FIELD");
			return NULL;
		case MAKE_OPCODE(AML_OPCODE_CREATE_BYTE_FIELD,0):
			panic("Unimplemented: AML_OPCODE_CREATE_BYTE_FIELD");
			return NULL;
		case MAKE_OPCODE(AML_OPCODE_CREATE_BIT_FIELD,0):
			panic("Unimplemented: AML_OPCODE_CREATE_BIT_FIELD");
			return NULL;
		case MAKE_OPCODE(AML_OPCODE_OBJECT_TYPE,0):
			panic("Unimplemented: AML_OPCODE_OBJECT_TYPE");
			return NULL;
		case MAKE_OPCODE(AML_OPCODE_CREATE_QWORD_FIELD,0):
			panic("Unimplemented: AML_OPCODE_CREATE_QWORD_FIELD");
			return NULL;
		case MAKE_OPCODE(AML_OPCODE_L_AND,0):
			panic("Unimplemented: AML_OPCODE_L_AND");
			return NULL;
		case MAKE_OPCODE(AML_OPCODE_L_OR,0):
			local->simple_return_value.type=AML_NODE_TYPE_INTEGER;
			local->simple_return_value.data.integer=(!!_get_arg_as_int(local,object,0))||(!!_get_arg_as_int(local,object,1));
			return &(local->simple_return_value);
		case MAKE_OPCODE(AML_OPCODE_L_NOT,0):
			panic("Unimplemented: AML_OPCODE_L_NOT");
			return NULL;
		case MAKE_OPCODE(AML_OPCODE_L_EQUAL,0):
			local->simple_return_value.type=AML_NODE_TYPE_INTEGER;
			local->simple_return_value.data.integer=(_get_arg_as_int(local,object,0)==_get_arg_as_int(local,object,1));
			return &(local->simple_return_value);
		case MAKE_OPCODE(AML_OPCODE_L_GREATER,0):
			panic("Unimplemented: AML_OPCODE_L_GREATER");
			return NULL;
		case MAKE_OPCODE(AML_OPCODE_L_LESS,0):
			panic("Unimplemented: AML_OPCODE_L_LESS");
			return NULL;
		case MAKE_OPCODE(AML_OPCODE_TO_BUFFER,0):
			panic("Unimplemented: AML_OPCODE_TO_BUFFER");
			return NULL;
		case MAKE_OPCODE(AML_OPCODE_TO_DECIMAL_STRING,0):
			panic("Unimplemented: AML_OPCODE_TO_DECIMAL_STRING");
			return NULL;
		case MAKE_OPCODE(AML_OPCODE_TO_HEX_STRING,0):
			panic("Unimplemented: AML_OPCODE_TO_HEX_STRING");
			return NULL;
		case MAKE_OPCODE(AML_OPCODE_TO_INTEGER,0):
			panic("Unimplemented: AML_OPCODE_TO_INTEGER");
			return NULL;
		case MAKE_OPCODE(AML_OPCODE_TO_STRING,0):
			panic("Unimplemented: AML_OPCODE_TO_STRING");
			return NULL;
		case MAKE_OPCODE(AML_OPCODE_COPY_OBJECT,0):
			panic("Unimplemented: AML_OPCODE_COPY_OBJECT");
			return NULL;
		case MAKE_OPCODE(AML_OPCODE_MID,0):
			panic("Unimplemented: AML_OPCODE_MID");
			return NULL;
		case MAKE_OPCODE(AML_OPCODE_CONTINUE,0):
			panic("Unimplemented: AML_OPCODE_CONTINUE");
			return NULL;
		case MAKE_OPCODE(AML_OPCODE_IF,0):
			local->was_branch_taken=!!_get_arg_as_int(local,object,0);
			if (local->was_branch_taken){
				_execute_multiple(local,object->data.child,object->data_length);
			}
			return NULL;
		case MAKE_OPCODE(AML_OPCODE_ELSE,0):
			if (!local->was_branch_taken){
				_execute_multiple(local,object->data.child,object->data_length);
			}
			return NULL;
		case MAKE_OPCODE(AML_OPCODE_WHILE,0):
			panic("Unimplemented: AML_OPCODE_WHILE");
			return NULL;
		case MAKE_OPCODE(AML_OPCODE_NOOP,0):
			panic("Unimplemented: AML_OPCODE_NOOP");
			return NULL;
		case MAKE_OPCODE(AML_OPCODE_RETURN,0):
			{
				aml_node_t* value=_get_arg_as_node(local,object,0);
				if (!local->return_value){
					local->return_value=value;
				}
				return NULL;
			}
		case MAKE_OPCODE(AML_OPCODE_BREAK,0):
			panic("Unimplemented: AML_OPCODE_BREAK");
			return NULL;
		case MAKE_OPCODE(AML_OPCODE_BREAK_POINT,0):
			panic("Unimplemented: AML_OPCODE_BREAK_POINT");
			return NULL;
		case MAKE_OPCODE(AML_OPCODE_ONES,0):
			local->simple_return_value.type=AML_NODE_TYPE_INTEGER;
			local->simple_return_value.data.integer=0xffffffffffffffffull;
			return &(local->simple_return_value);
		case MAKE_OPCODE(AML_EXTENDED_OPCODE,AML_OPCODE_EXT_MUTEX):
			{
				aml_node_t* mutex=_get_node(local->namespace,object->args[0].string,AML_NODE_TYPE_MUTEX,1);
				return mutex;
			}
		case MAKE_OPCODE(AML_EXTENDED_OPCODE,AML_OPCODE_EXT_EVENT):
			panic("Unimplemented: AML_OPCODE_EXT_EVENT");
			return NULL;
		case MAKE_OPCODE(AML_EXTENDED_OPCODE,AML_OPCODE_EXT_COND_REF_OF):
			panic("Unimplemented: AML_OPCODE_EXT_COND_REF_OF");
			return NULL;
		case MAKE_OPCODE(AML_EXTENDED_OPCODE,AML_OPCODE_EXT_CREATE_FIELD):
			panic("Unimplemented: AML_OPCODE_EXT_CREATE_FIELD");
			return NULL;
		case MAKE_OPCODE(AML_EXTENDED_OPCODE,AML_OPCODE_EXT_LOAD_TABLE):
			panic("Unimplemented: AML_OPCODE_EXT_LOAD_TABLE");
			return NULL;
		case MAKE_OPCODE(AML_EXTENDED_OPCODE,AML_OPCODE_EXT_LOAD):
			panic("Unimplemented: AML_OPCODE_EXT_LOAD");
			return NULL;
		case MAKE_OPCODE(AML_EXTENDED_OPCODE,AML_OPCODE_EXT_STALL):
			panic("Unimplemented: AML_OPCODE_EXT_STALL");
			return NULL;
		case MAKE_OPCODE(AML_EXTENDED_OPCODE,AML_OPCODE_EXT_SLEEP):
			panic("Unimplemented: AML_OPCODE_EXT_SLEEP");
			return NULL;
		case MAKE_OPCODE(AML_EXTENDED_OPCODE,AML_OPCODE_EXT_ACQUIRE):
			panic("Unimplemented: AML_OPCODE_EXT_ACQUIRE");
			return NULL;
		case MAKE_OPCODE(AML_EXTENDED_OPCODE,AML_OPCODE_EXT_SIGNAL):
			panic("Unimplemented: AML_OPCODE_EXT_SIGNAL");
			return NULL;
		case MAKE_OPCODE(AML_EXTENDED_OPCODE,AML_OPCODE_EXT_WAIT):
			panic("Unimplemented: AML_OPCODE_EXT_WAIT");
			return NULL;
		case MAKE_OPCODE(AML_EXTENDED_OPCODE,AML_OPCODE_EXT_RESET):
			panic("Unimplemented: AML_OPCODE_EXT_RESET");
			return NULL;
		case MAKE_OPCODE(AML_EXTENDED_OPCODE,AML_OPCODE_EXT_RELEASE):
			panic("Unimplemented: AML_OPCODE_EXT_RELEASE");
			return NULL;
		case MAKE_OPCODE(AML_EXTENDED_OPCODE,AML_OPCODE_EXT_FROM_BCD):
			panic("Unimplemented: AML_OPCODE_EXT_FROM_BCD");
			return NULL;
		case MAKE_OPCODE(AML_EXTENDED_OPCODE,AML_OPCODE_EXT_TO_BCD):
			panic("Unimplemented: AML_OPCODE_EXT_TO_BCD");
			return NULL;
		case MAKE_OPCODE(AML_EXTENDED_OPCODE,AML_OPCODE_EXT_UNLOAD):
			panic("Unimplemented: AML_OPCODE_EXT_UNLOAD");
			return NULL;
		case MAKE_OPCODE(AML_EXTENDED_OPCODE,AML_OPCODE_EXT_REVISION):
			panic("Unimplemented: AML_OPCODE_EXT_REVISION");
			return NULL;
		case MAKE_OPCODE(AML_EXTENDED_OPCODE,AML_OPCODE_EXT_DEBUG):
			panic("Unimplemented: AML_OPCODE_EXT_DEBUG");
			return NULL;
		case MAKE_OPCODE(AML_EXTENDED_OPCODE,AML_OPCODE_EXT_FATAL):
			panic("Unimplemented: AML_OPCODE_EXT_FATAL");
			return NULL;
		case MAKE_OPCODE(AML_EXTENDED_OPCODE,AML_OPCODE_EXT_TIMER):
			panic("Unimplemented: AML_OPCODE_EXT_TIMER");
			return NULL;
		case MAKE_OPCODE(AML_EXTENDED_OPCODE,AML_OPCODE_EXT_REGION):
			{
				aml_node_t* region=_get_node(local->namespace,object->args[0].string,AML_NODE_TYPE_REGION,1);
				region->data.region.type=object->args[1].number;
				region->data.region.offset=_get_arg_as_int(local,object,2);
				region->data.region.length=_get_arg_as_int(local,object,3);
				return region;
			}
		case MAKE_OPCODE(AML_EXTENDED_OPCODE,AML_OPCODE_EXT_FIELD):
			{
				aml_node_t* region=_get_node(local->namespace,object->args[0].string,AML_NODE_TYPE_REGION,1);
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
						panic("Unimplemented");
					}
					aml_node_t* field_unit=_alloc_node((const char*)(object->data.bytes+i),AML_NODE_TYPE_FIELD_UNIT,local->namespace);
					i+=4;
					u32 size;
					i+=aml_parse_pkglength(object->data.bytes+i,&size);
					field_unit->data.field_unit.type=region_type;
					field_unit->data.field_unit.access_type=object->args[1].number;
					spinlock_init(&(field_unit->data.field_unit.lock));
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
				aml_node_t* device=_get_node(local->namespace,object->args[0].string,AML_NODE_TYPE_DEVICE,1);
				aml_node_t* prev_namespace=local->namespace;
				local->namespace=device;
				_execute_multiple(local,object->data.child,object->data_length);
				local->namespace=prev_namespace;
				return device;
			}
		case MAKE_OPCODE(AML_EXTENDED_OPCODE,AML_OPCODE_EXT_PROCESSOR):
			{
				aml_node_t* processor=_get_node(local->namespace,object->args[0].string,AML_NODE_TYPE_PROCESSOR,1);
				processor->data.processor.id=object->args[1].number;
				processor->data.processor.block_address=object->args[2].number;
				processor->data.processor.block_length=object->args[3].number;
				aml_node_t* prev_namespace=local->namespace;
				local->namespace=processor;
				_execute_multiple(local,object->data.child,object->data_length);
				local->namespace=prev_namespace;
				return processor;
			}
		case MAKE_OPCODE(AML_EXTENDED_OPCODE,AML_OPCODE_EXT_POWER_RES):
			panic("Unimplemented: AML_OPCODE_EXT_POWER_RES");
			return NULL;
		case MAKE_OPCODE(AML_EXTENDED_OPCODE,AML_OPCODE_EXT_THERMAL_ZONE):
			panic("Unimplemented: AML_OPCODE_EXT_THERMAL_ZONE");
			return NULL;
		case MAKE_OPCODE(AML_EXTENDED_OPCODE,AML_OPCODE_EXT_INDEX_FIELD):
			panic("Unimplemented: AML_OPCODE_EXT_INDEX_FIELD");
			return NULL;
		case MAKE_OPCODE(AML_EXTENDED_OPCODE,AML_OPCODE_EXT_BANK_FIELD):
			panic("Unimplemented: AML_OPCODE_EXT_BANK_FIELD");
			return NULL;
		case MAKE_OPCODE(AML_EXTENDED_OPCODE,AML_OPCODE_EXT_DATA_REGION):
			panic("Unimplemented: AML_OPCODE_EXT_DATA_REGION");
			return NULL;
		case MAKE_OPCODE(AML_OPCODE_NAME_REFERENCE,0):
			for (aml_node_t* namespace=local->namespace;1;namespace=namespace->parent){
				aml_node_t* out=_get_node(namespace,object->args[0].string,0,0);
				if (out){
					return out;
				}
				if (namespace==namespace->parent){
					break;
				}
			}
			ERROR("Object not found: %s",object->args[0].string);
			panic("Object not found");
			return NULL;
	}
	return NULL;
}



static void _execute_multiple(runtime_local_state_t* local,const aml_object_t* first_object,u32 object_count){
	_Bool prev_was_branch_taken=local->was_branch_taken;
	local->was_branch_taken=0;
	for (u32 i=0;i<object_count&&!local->return_value;i++){
		_execute(local,first_object);
		first_object=first_object->next;
	}
	local->was_branch_taken=prev_was_branch_taken;
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
			goto _end;
		case AML_NODE_TYPE_BUFFER:
			log("buffer<size=%u>",node->data.buffer->length);
			if (node->data.buffer->length>16){
				goto _end;
			}
			log("{",node->data.buffer->length);
			for (u8 i=0;i<node->data.buffer->length;i++){
				if (i){
					log(" ");
				}
				log("%x",node->data.buffer->data[i]);
			}
			log("}");
			goto _end;
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
			goto _end;
		case AML_NODE_TYPE_FIELD_UNIT:
			log("field_unit<address=%p, size=%u>",node->data.field_unit.address,node->data.field_unit.size);
			goto _end;
		case AML_NODE_TYPE_INTEGER:
			log("0x%lx",node->data.integer);
			goto _end;
		case AML_NODE_TYPE_METHOD:
			log("method<arg_count=%u>",node->data.method.flags&7);
			goto _end;
		case AML_NODE_TYPE_MUTEX:
			log("mutex<...>");
			goto _end;
		case AML_NODE_TYPE_REFERENCE:
			log("reference<...>");
			goto _end;
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
			goto _end;
		case AML_NODE_TYPE_POWER_RESOURCE:
			log("power_resource<...>");
			break;
		case AML_NODE_TYPE_PROCESSOR:
			log("processor<id=%u>",node->data.processor.id);
			break;
		case AML_NODE_TYPE_STRING:
			log("'%s'",node->data.string->data);
			goto _end;
		case AML_NODE_TYPE_THERMAL_ZONE:
			log("thermal_zone<...>");
			break;
	}
	if ((node->type==AML_NODE_TYPE_PACKAGE?!node->data.package.length:!node->child)){
		log("{}");
		goto _end;
	}
	log("{\n");
	if (node->type==AML_NODE_TYPE_PACKAGE){
		for (aml_node_t* child=node->data.package.child;child;child=child->next){
			if (child==node->data.package.child){
				log(",");
			}
			_print_node(child,indent+4,1);
			log("\n");
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
_end:
	if (!indent){
		log("\n");
	}
}



void aml_runtime_init(aml_object_t* root,u16 irq){
	LOG("Building AML runtime...");
	aml_root_node=_alloc_node("\\\x00\x00\x00",AML_NODE_TYPE_SCOPE,NULL);
	runtime_local_state_t local={
		aml_root_node
	};
	local.simple_return_value.flags=AML_NODE_FLAG_LOCAL;
	_execute_multiple(&local,root->data.child,root->data_length);
	INFO("Registering AML IRQ...");
	ioapic_redirect_irq(irq,isr_allocate());
}



aml_node_t* aml_runtime_get_node(aml_node_t* root,const char* path){
	string_t* path_data=smm_alloc(path,0);
	aml_node_t* out=_get_node((root?root:aml_root_node),path_data,0,0);
	smm_dealloc(path_data);
	return out;
}



aml_node_t* aml_runtime_evaluate_node(aml_node_t* node){
	if (!node||node->type!=AML_NODE_TYPE_METHOD){
		return node;
	}
	runtime_local_state_t local;
	local.namespace=node;
	local.return_value=NULL;
	_execute_multiple(&local,node->data.method.child,node->data.method.child_count);
	return local.return_value; // Can return stack-local pointer!
}



void aml_runtime_print_node(aml_node_t* node){
	_print_node(node,0,0);
}
