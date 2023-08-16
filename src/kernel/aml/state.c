#include <kernel/aml/aml.h>
#include <kernel/kernel.h>
#include <kernel/log/log.h>
#include <kernel/memory/kmm.h>
#include <kernel/types.h>
#define KERNEL_LOG_NAME "aml"



#define MAKE_OPCODE(a,b) ((a)|((b)<<8))



typedef struct _NAMESPACE{
	char name[5];
	struct _NAMESPACE* parent;
	struct _NAMESPACE* child;
	struct _NAMESPACE* next;
} namespace_t;


typedef struct _RUNTIME_GLOBAL_STATE{
	namespace_t* root_namespace;
} runtime_global_state_t;



typedef struct _RUNTIME_LOCAL_STATE{
	namespace_t* namespace;
	u64 args[7];
	u64 locals[8];
} runtime_local_state_t;



static namespace_t* _alloc_namespace(const char* name,namespace_t* parent){
	namespace_t* out=kmm_allocate(sizeof(namespace_t));
	for (u8 i=0;i<4;i++){
		out->name[i]=name[i];
	}
	out->name[4]=0;
	out->parent=parent;
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



static namespace_t* _get_namespace(runtime_global_state_t* global,runtime_local_state_t* local,const char* namespace){
	namespace_t* out=local->namespace;
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
			for (namespace_t* child=out->child;child;child=child->next){
				if (*((const u32*)(child->name))==*((const u32*)namespace)){
					out=child;
					goto _namespace_found;
				}
			}
			out=_alloc_namespace(namespace,out);
_namespace_found:
			namespace+=4;
		}
	}
	return out;
}



static void _execute(runtime_global_state_t* global,runtime_local_state_t* local,const aml_object_t* objects,u32 object_count){
	for (u32 i=0;i<object_count;i++){
		const aml_object_t* object=objects+i;
		switch (MAKE_OPCODE(object->opcode[0],object->opcode[1])){
			case MAKE_OPCODE(AML_OPCODE_ZERO,0):
				ERROR("Unimplemented: AML_OPCODE_ZERO");for (;;);
				break;
			case MAKE_OPCODE(AML_OPCODE_ONE,0):
				ERROR("Unimplemented: AML_OPCODE_ONE");for (;;);
				break;
			case MAKE_OPCODE(AML_OPCODE_ALIAS,0):
				ERROR("Unimplemented: AML_OPCODE_ALIAS");for (;;);
				break;
			case MAKE_OPCODE(AML_OPCODE_NAME,0):
				ERROR("Unimplemented: AML_OPCODE_NAME");for (;;);
				break;
			case MAKE_OPCODE(AML_OPCODE_BYTE_PREFIX,0):
				ERROR("Unimplemented: AML_OPCODE_BYTE_PREFIX");for (;;);
				break;
			case MAKE_OPCODE(AML_OPCODE_WORD_PREFIX,0):
				ERROR("Unimplemented: AML_OPCODE_WORD_PREFIX");for (;;);
				break;
			case MAKE_OPCODE(AML_OPCODE_DWORD_PREFIX,0):
				ERROR("Unimplemented: AML_OPCODE_DWORD_PREFIX");for (;;);
				break;
			case MAKE_OPCODE(AML_OPCODE_STRING_PREFIX,0):
				ERROR("Unimplemented: AML_OPCODE_STRING_PREFIX");for (;;);
				break;
			case MAKE_OPCODE(AML_OPCODE_QWORD_PREFIX,0):
				ERROR("Unimplemented: AML_OPCODE_QWORD_PREFIX");for (;;);
				break;
			case MAKE_OPCODE(AML_OPCODE_SCOPE,0):
				{
					namespace_t* prev_namespace=local->namespace;
					local->namespace=_get_namespace(global,local,object->args[0].string);
					_execute(global,local,object->data.objects,object->data_length);
					local->namespace=prev_namespace;
					break;
				}
			case MAKE_OPCODE(AML_OPCODE_BUFFER,0):
				ERROR("Unimplemented: AML_OPCODE_BUFFER");for (;;);
				break;
			case MAKE_OPCODE(AML_OPCODE_PACKAGE,0):
				ERROR("Unimplemented: AML_OPCODE_PACKAGE");for (;;);
				break;
			case MAKE_OPCODE(AML_OPCODE_VAR_PACKAGE,0):
				ERROR("Unimplemented: AML_OPCODE_VAR_PACKAGE");for (;;);
				break;
			case MAKE_OPCODE(AML_OPCODE_METHOD,0):
				ERROR("Unimplemented: AML_OPCODE_METHOD");for (;;);
				break;
			case MAKE_OPCODE(AML_OPCODE_LOCAL0,0):
				ERROR("Unimplemented: AML_OPCODE_LOCAL0");for (;;);
				break;
			case MAKE_OPCODE(AML_OPCODE_LOCAL1,0):
				ERROR("Unimplemented: AML_OPCODE_LOCAL1");for (;;);
				break;
			case MAKE_OPCODE(AML_OPCODE_LOCAL2,0):
				ERROR("Unimplemented: AML_OPCODE_LOCAL2");for (;;);
				break;
			case MAKE_OPCODE(AML_OPCODE_LOCAL3,0):
				ERROR("Unimplemented: AML_OPCODE_LOCAL3");for (;;);
				break;
			case MAKE_OPCODE(AML_OPCODE_LOCAL4,0):
				ERROR("Unimplemented: AML_OPCODE_LOCAL4");for (;;);
				break;
			case MAKE_OPCODE(AML_OPCODE_LOCAL5,0):
				ERROR("Unimplemented: AML_OPCODE_LOCAL5");for (;;);
				break;
			case MAKE_OPCODE(AML_OPCODE_LOCAL6,0):
				ERROR("Unimplemented: AML_OPCODE_LOCAL6");for (;;);
				break;
			case MAKE_OPCODE(AML_OPCODE_LOCAL7,0):
				ERROR("Unimplemented: AML_OPCODE_LOCAL7");for (;;);
				break;
			case MAKE_OPCODE(AML_OPCODE_ARG0,0):
				ERROR("Unimplemented: AML_OPCODE_ARG0");for (;;);
				break;
			case MAKE_OPCODE(AML_OPCODE_ARG1,0):
				ERROR("Unimplemented: AML_OPCODE_ARG1");for (;;);
				break;
			case MAKE_OPCODE(AML_OPCODE_ARG2,0):
				ERROR("Unimplemented: AML_OPCODE_ARG2");for (;;);
				break;
			case MAKE_OPCODE(AML_OPCODE_ARG3,0):
				ERROR("Unimplemented: AML_OPCODE_ARG3");for (;;);
				break;
			case MAKE_OPCODE(AML_OPCODE_ARG4,0):
				ERROR("Unimplemented: AML_OPCODE_ARG4");for (;;);
				break;
			case MAKE_OPCODE(AML_OPCODE_ARG5,0):
				ERROR("Unimplemented: AML_OPCODE_ARG5");for (;;);
				break;
			case MAKE_OPCODE(AML_OPCODE_ARG6,0):
				ERROR("Unimplemented: AML_OPCODE_ARG6");for (;;);
				break;
			case MAKE_OPCODE(AML_OPCODE_STORE,0):
				ERROR("Unimplemented: AML_OPCODE_STORE");for (;;);
				break;
			case MAKE_OPCODE(AML_OPCODE_REF_OF,0):
				ERROR("Unimplemented: AML_OPCODE_REF_OF");for (;;);
				break;
			case MAKE_OPCODE(AML_OPCODE_ADD,0):
				ERROR("Unimplemented: AML_OPCODE_ADD");for (;;);
				break;
			case MAKE_OPCODE(AML_OPCODE_CONCAT,0):
				ERROR("Unimplemented: AML_OPCODE_CONCAT");for (;;);
				break;
			case MAKE_OPCODE(AML_OPCODE_SUBTRACT,0):
				ERROR("Unimplemented: AML_OPCODE_SUBTRACT");for (;;);
				break;
			case MAKE_OPCODE(AML_OPCODE_INCREMENT,0):
				ERROR("Unimplemented: AML_OPCODE_INCREMENT");for (;;);
				break;
			case MAKE_OPCODE(AML_OPCODE_DECREMENT,0):
				ERROR("Unimplemented: AML_OPCODE_DECREMENT");for (;;);
				break;
			case MAKE_OPCODE(AML_OPCODE_MULTIPLY,0):
				ERROR("Unimplemented: AML_OPCODE_MULTIPLY");for (;;);
				break;
			case MAKE_OPCODE(AML_OPCODE_DIVIDE,0):
				ERROR("Unimplemented: AML_OPCODE_DIVIDE");for (;;);
				break;
			case MAKE_OPCODE(AML_OPCODE_SHIFT_LEFT,0):
				ERROR("Unimplemented: AML_OPCODE_SHIFT_LEFT");for (;;);
				break;
			case MAKE_OPCODE(AML_OPCODE_SHIFT_RIGHT,0):
				ERROR("Unimplemented: AML_OPCODE_SHIFT_RIGHT");for (;;);
				break;
			case MAKE_OPCODE(AML_OPCODE_AND,0):
				ERROR("Unimplemented: AML_OPCODE_AND");for (;;);
				break;
			case MAKE_OPCODE(AML_OPCODE_NAND,0):
				ERROR("Unimplemented: AML_OPCODE_NAND");for (;;);
				break;
			case MAKE_OPCODE(AML_OPCODE_OR,0):
				ERROR("Unimplemented: AML_OPCODE_OR");for (;;);
				break;
			case MAKE_OPCODE(AML_OPCODE_NOR,0):
				ERROR("Unimplemented: AML_OPCODE_NOR");for (;;);
				break;
			case MAKE_OPCODE(AML_OPCODE_XOR,0):
				ERROR("Unimplemented: AML_OPCODE_XOR");for (;;);
				break;
			case MAKE_OPCODE(AML_OPCODE_NOT,0):
				ERROR("Unimplemented: AML_OPCODE_NOT");for (;;);
				break;
			case MAKE_OPCODE(AML_OPCODE_FIND_SET_LEFT_BIT,0):
				ERROR("Unimplemented: AML_OPCODE_FIND_SET_LEFT_BIT");for (;;);
				break;
			case MAKE_OPCODE(AML_OPCODE_FIND_SET_RIGHT_BIT,0):
				ERROR("Unimplemented: AML_OPCODE_FIND_SET_RIGHT_BIT");for (;;);
				break;
			case MAKE_OPCODE(AML_OPCODE_DEREF_OF,0):
				ERROR("Unimplemented: AML_OPCODE_DEREF_OF");for (;;);
				break;
			case MAKE_OPCODE(AML_OPCODE_CONCAT_RES,0):
				ERROR("Unimplemented: AML_OPCODE_CONCAT_RES");for (;;);
				break;
			case MAKE_OPCODE(AML_OPCODE_MOD,0):
				ERROR("Unimplemented: AML_OPCODE_MOD");for (;;);
				break;
			case MAKE_OPCODE(AML_OPCODE_NOTIFY,0):
				ERROR("Unimplemented: AML_OPCODE_NOTIFY");for (;;);
				break;
			case MAKE_OPCODE(AML_OPCODE_SIZE_OF,0):
				ERROR("Unimplemented: AML_OPCODE_SIZE_OF");for (;;);
				break;
			case MAKE_OPCODE(AML_OPCODE_INDEX,0):
				ERROR("Unimplemented: AML_OPCODE_INDEX");for (;;);
				break;
			case MAKE_OPCODE(AML_OPCODE_MATCH,0):
				ERROR("Unimplemented: AML_OPCODE_MATCH");for (;;);
				break;
			case MAKE_OPCODE(AML_OPCODE_CREATE_DWORD_FIELD,0):
				ERROR("Unimplemented: AML_OPCODE_CREATE_DWORD_FIELD");for (;;);
				break;
			case MAKE_OPCODE(AML_OPCODE_CREATE_WORD_FIELD,0):
				ERROR("Unimplemented: AML_OPCODE_CREATE_WORD_FIELD");for (;;);
				break;
			case MAKE_OPCODE(AML_OPCODE_CREATE_BYTE_FIELD,0):
				ERROR("Unimplemented: AML_OPCODE_CREATE_BYTE_FIELD");for (;;);
				break;
			case MAKE_OPCODE(AML_OPCODE_CREATE_BIT_FIELD,0):
				ERROR("Unimplemented: AML_OPCODE_CREATE_BIT_FIELD");for (;;);
				break;
			case MAKE_OPCODE(AML_OPCODE_OBJECT_TYPE,0):
				ERROR("Unimplemented: AML_OPCODE_OBJECT_TYPE");for (;;);
				break;
			case MAKE_OPCODE(AML_OPCODE_CREATE_QWORD_FIELD,0):
				ERROR("Unimplemented: AML_OPCODE_CREATE_QWORD_FIELD");for (;;);
				break;
			case MAKE_OPCODE(AML_OPCODE_L_AND,0):
				ERROR("Unimplemented: AML_OPCODE_L_AND");for (;;);
				break;
			case MAKE_OPCODE(AML_OPCODE_L_OR,0):
				ERROR("Unimplemented: AML_OPCODE_L_OR");for (;;);
				break;
			case MAKE_OPCODE(AML_OPCODE_L_NOT,0):
				ERROR("Unimplemented: AML_OPCODE_L_NOT");for (;;);
				break;
			case MAKE_OPCODE(AML_OPCODE_L_EQUAL,0):
				ERROR("Unimplemented: AML_OPCODE_L_EQUAL");for (;;);
				break;
			case MAKE_OPCODE(AML_OPCODE_L_GREATER,0):
				ERROR("Unimplemented: AML_OPCODE_L_GREATER");for (;;);
				break;
			case MAKE_OPCODE(AML_OPCODE_L_LESS,0):
				ERROR("Unimplemented: AML_OPCODE_L_LESS");for (;;);
				break;
			case MAKE_OPCODE(AML_OPCODE_TO_BUFFER,0):
				ERROR("Unimplemented: AML_OPCODE_TO_BUFFER");for (;;);
				break;
			case MAKE_OPCODE(AML_OPCODE_TO_DECIMAL_STRING,0):
				ERROR("Unimplemented: AML_OPCODE_TO_DECIMAL_STRING");for (;;);
				break;
			case MAKE_OPCODE(AML_OPCODE_TO_HEX_STRING,0):
				ERROR("Unimplemented: AML_OPCODE_TO_HEX_STRING");for (;;);
				break;
			case MAKE_OPCODE(AML_OPCODE_TO_INTEGER,0):
				ERROR("Unimplemented: AML_OPCODE_TO_INTEGER");for (;;);
				break;
			case MAKE_OPCODE(AML_OPCODE_TO_STRING,0):
				ERROR("Unimplemented: AML_OPCODE_TO_STRING");for (;;);
				break;
			case MAKE_OPCODE(AML_OPCODE_COPY_OBJECT,0):
				ERROR("Unimplemented: AML_OPCODE_COPY_OBJECT");for (;;);
				break;
			case MAKE_OPCODE(AML_OPCODE_MID,0):
				ERROR("Unimplemented: AML_OPCODE_MID");for (;;);
				break;
			case MAKE_OPCODE(AML_OPCODE_CONTINUE,0):
				ERROR("Unimplemented: AML_OPCODE_CONTINUE");for (;;);
				break;
			case MAKE_OPCODE(AML_OPCODE_IF,0):
				ERROR("Unimplemented: AML_OPCODE_IF");for (;;);
				break;
			case MAKE_OPCODE(AML_OPCODE_ELSE,0):
				ERROR("Unimplemented: AML_OPCODE_ELSE");for (;;);
				break;
			case MAKE_OPCODE(AML_OPCODE_WHILE,0):
				ERROR("Unimplemented: AML_OPCODE_WHILE");for (;;);
				break;
			case MAKE_OPCODE(AML_OPCODE_NOOP,0):
				ERROR("Unimplemented: AML_OPCODE_NOOP");for (;;);
				break;
			case MAKE_OPCODE(AML_OPCODE_RETURN,0):
				ERROR("Unimplemented: AML_OPCODE_RETURN");for (;;);
				break;
			case MAKE_OPCODE(AML_OPCODE_BREAK,0):
				ERROR("Unimplemented: AML_OPCODE_BREAK");for (;;);
				break;
			case MAKE_OPCODE(AML_OPCODE_BREAK_POINT,0):
				ERROR("Unimplemented: AML_OPCODE_BREAK_POINT");for (;;);
				break;
			case MAKE_OPCODE(AML_OPCODE_ONES,0):
				ERROR("Unimplemented: AML_OPCODE_ONES");for (;;);
				break;
			case MAKE_OPCODE(AML_EXTENDED_OPCODE,AML_OPCODE_EXT_MUTEX):
				ERROR("Unimplemented: AML_OPCODE_EXT_MUTEX");for (;;);
				break;
			case MAKE_OPCODE(AML_EXTENDED_OPCODE,AML_OPCODE_EXT_EVENT):
				ERROR("Unimplemented: AML_OPCODE_EXT_EVENT");for (;;);
				break;
			case MAKE_OPCODE(AML_EXTENDED_OPCODE,AML_OPCODE_EXT_COND_REF_OF):
				ERROR("Unimplemented: AML_OPCODE_EXT_COND_REF_OF");for (;;);
				break;
			case MAKE_OPCODE(AML_EXTENDED_OPCODE,AML_OPCODE_EXT_CREATE_FIELD):
				ERROR("Unimplemented: AML_OPCODE_EXT_CREATE_FIELD");for (;;);
				break;
			case MAKE_OPCODE(AML_EXTENDED_OPCODE,AML_OPCODE_EXT_LOAD_TABLE):
				ERROR("Unimplemented: AML_OPCODE_EXT_LOAD_TABLE");for (;;);
				break;
			case MAKE_OPCODE(AML_EXTENDED_OPCODE,AML_OPCODE_EXT_LOAD):
				ERROR("Unimplemented: AML_OPCODE_EXT_LOAD");for (;;);
				break;
			case MAKE_OPCODE(AML_EXTENDED_OPCODE,AML_OPCODE_EXT_STALL):
				ERROR("Unimplemented: AML_OPCODE_EXT_STALL");for (;;);
				break;
			case MAKE_OPCODE(AML_EXTENDED_OPCODE,AML_OPCODE_EXT_SLEEP):
				ERROR("Unimplemented: AML_OPCODE_EXT_SLEEP");for (;;);
				break;
			case MAKE_OPCODE(AML_EXTENDED_OPCODE,AML_OPCODE_EXT_ACQUIRE):
				ERROR("Unimplemented: AML_OPCODE_EXT_ACQUIRE");for (;;);
				break;
			case MAKE_OPCODE(AML_EXTENDED_OPCODE,AML_OPCODE_EXT_SIGNAL):
				ERROR("Unimplemented: AML_OPCODE_EXT_SIGNAL");for (;;);
				break;
			case MAKE_OPCODE(AML_EXTENDED_OPCODE,AML_OPCODE_EXT_WAIT):
				ERROR("Unimplemented: AML_OPCODE_EXT_WAIT");for (;;);
				break;
			case MAKE_OPCODE(AML_EXTENDED_OPCODE,AML_OPCODE_EXT_RESET):
				ERROR("Unimplemented: AML_OPCODE_EXT_RESET");for (;;);
				break;
			case MAKE_OPCODE(AML_EXTENDED_OPCODE,AML_OPCODE_EXT_RELEASE):
				ERROR("Unimplemented: AML_OPCODE_EXT_RELEASE");for (;;);
				break;
			case MAKE_OPCODE(AML_EXTENDED_OPCODE,AML_OPCODE_EXT_FROM_BCD):
				ERROR("Unimplemented: AML_OPCODE_EXT_FROM_BCD");for (;;);
				break;
			case MAKE_OPCODE(AML_EXTENDED_OPCODE,AML_OPCODE_EXT_TO_BCD):
				ERROR("Unimplemented: AML_OPCODE_EXT_TO_BCD");for (;;);
				break;
			case MAKE_OPCODE(AML_EXTENDED_OPCODE,AML_OPCODE_EXT_UNLOAD):
				ERROR("Unimplemented: AML_OPCODE_EXT_UNLOAD");for (;;);
				break;
			case MAKE_OPCODE(AML_EXTENDED_OPCODE,AML_OPCODE_EXT_REVISION):
				ERROR("Unimplemented: AML_OPCODE_EXT_REVISION");for (;;);
				break;
			case MAKE_OPCODE(AML_EXTENDED_OPCODE,AML_OPCODE_EXT_DEBUG):
				ERROR("Unimplemented: AML_OPCODE_EXT_DEBUG");for (;;);
				break;
			case MAKE_OPCODE(AML_EXTENDED_OPCODE,AML_OPCODE_EXT_FATAL):
				ERROR("Unimplemented: AML_OPCODE_EXT_FATAL");for (;;);
				break;
			case MAKE_OPCODE(AML_EXTENDED_OPCODE,AML_OPCODE_EXT_TIMER):
				ERROR("Unimplemented: AML_OPCODE_EXT_TIMER");for (;;);
				break;
			case MAKE_OPCODE(AML_EXTENDED_OPCODE,AML_OPCODE_EXT_REGION):
				ERROR("Unimplemented: AML_OPCODE_EXT_REGION");for (;;);
				break;
			case MAKE_OPCODE(AML_EXTENDED_OPCODE,AML_OPCODE_EXT_FIELD):
				ERROR("Unimplemented: AML_OPCODE_EXT_FIELD");for (;;);
				break;
			case MAKE_OPCODE(AML_EXTENDED_OPCODE,AML_OPCODE_EXT_DEVICE):
				ERROR("Unimplemented: AML_OPCODE_EXT_DEVICE");for (;;);
				break;
			case MAKE_OPCODE(AML_EXTENDED_OPCODE,AML_OPCODE_EXT_PROCESSOR):
				ERROR("Unimplemented: AML_OPCODE_EXT_PROCESSOR");for (;;);
				break;
			case MAKE_OPCODE(AML_EXTENDED_OPCODE,AML_OPCODE_EXT_POWER_RES):
				ERROR("Unimplemented: AML_OPCODE_EXT_POWER_RES");for (;;);
				break;
			case MAKE_OPCODE(AML_EXTENDED_OPCODE,AML_OPCODE_EXT_THERMAL_ZONE):
				ERROR("Unimplemented: AML_OPCODE_EXT_THERMAL_ZONE");for (;;);
				break;
			case MAKE_OPCODE(AML_EXTENDED_OPCODE,AML_OPCODE_EXT_INDEX_FIELD):
				ERROR("Unimplemented: AML_OPCODE_EXT_INDEX_FIELD");for (;;);
				break;
			case MAKE_OPCODE(AML_EXTENDED_OPCODE,AML_OPCODE_EXT_BANK_FIELD):
				ERROR("Unimplemented: AML_OPCODE_EXT_BANK_FIELD");for (;;);
				break;
			case MAKE_OPCODE(AML_EXTENDED_OPCODE,AML_OPCODE_EXT_DATA_REGION):
				ERROR("Unimplemented: AML_OPCODE_EXT_DATA_REGION");for (;;);
				break;
			case MAKE_OPCODE(AML_OPCODE_STRING,0):
				ERROR("Unimplemented: AML_OPCODE_STRING");for (;;);
				break;
		}
	}
}



void aml_build_runtime(aml_object_t* root){
	LOG("Building AML runtime...");
	runtime_global_state_t global={
		_alloc_namespace("\\\x00\x00\x00",NULL)
	};
	global.root_namespace->parent=global.root_namespace;
	runtime_local_state_t local={
		global.root_namespace
	};
	_execute(&global,&local,root->data.objects,root->data_length);
	for (;;);
}
