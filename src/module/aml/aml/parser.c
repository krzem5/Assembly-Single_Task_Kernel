// #include <kernel/aml/parser.h>
// #include <kernel/log/log.h>
// #include <kernel/memory/omm.h>
// #include <kernel/memory/pmm.h>
// #include <kernel/memory/vmm.h>
// #include <kernel/types.h>
// #include <kernel/util/util.h>
// #include <kernel/memory/smm.h>
// #define KERNEL_LOG_NAME "aml_parser"



// #define OPCODE_FLAG_EXTENDED 0x01
// #define OPCODE_FLAG_PKGLENGTH 0x02
// #define OPCODE_FLAG_EXTRA_BYTES 0x04



// #define _DECL_OPCODE_ARG_COUNT(_,_0,_1,_2,_3,_4,_5,n,...) n
// #define DECL_OPCODE(opcode,flags,...) {opcode,flags,_DECL_OPCODE_ARG_COUNT(_,##__VA_ARGS__,6,5,4,3,2,1,0),{__VA_ARGS__}}



// typedef struct _OPCODE{
// 	u16 opcode;
// 	u8 flags;
// 	u8 arg_count;
// 	u8 args[6];
// } opcode_t;



// static const opcode_t _aml_opcodes[]={
// 	DECL_OPCODE(AML_OPCODE_ZERO,0),
// 	DECL_OPCODE(AML_OPCODE_ONE,0),
// 	DECL_OPCODE(AML_OPCODE_ALIAS,0,AML_OBJECT_ARG_TYPE_NAME,AML_OBJECT_ARG_TYPE_NAME),
// 	DECL_OPCODE(AML_OPCODE_NAME,0,AML_OBJECT_ARG_TYPE_NAME,AML_OBJECT_ARG_TYPE_OBJECT),
// 	DECL_OPCODE(AML_OPCODE_BYTE_PREFIX,0,AML_OBJECT_ARG_TYPE_UINT8),
// 	DECL_OPCODE(AML_OPCODE_WORD_PREFIX,0,AML_OBJECT_ARG_TYPE_UINT16),
// 	DECL_OPCODE(AML_OPCODE_DWORD_PREFIX,0,AML_OBJECT_ARG_TYPE_UINT32),
// 	DECL_OPCODE(AML_OPCODE_NAME_REFERENCE_PREFIX,0,AML_OBJECT_ARG_TYPE_STRING),
// 	DECL_OPCODE(AML_OPCODE_QWORD_PREFIX,0,AML_OBJECT_ARG_TYPE_UINT64),
// 	DECL_OPCODE(AML_OPCODE_SCOPE,OPCODE_FLAG_PKGLENGTH,AML_OBJECT_ARG_TYPE_NAME),
// 	DECL_OPCODE(AML_OPCODE_BUFFER,OPCODE_FLAG_PKGLENGTH|OPCODE_FLAG_EXTRA_BYTES,AML_OBJECT_ARG_TYPE_OBJECT),
// 	DECL_OPCODE(AML_OPCODE_PACKAGE,OPCODE_FLAG_PKGLENGTH,AML_OBJECT_ARG_TYPE_UINT8),
// 	DECL_OPCODE(AML_OPCODE_VAR_PACKAGE,OPCODE_FLAG_PKGLENGTH,AML_OBJECT_ARG_TYPE_OBJECT),
// 	DECL_OPCODE(AML_OPCODE_METHOD,OPCODE_FLAG_PKGLENGTH,AML_OBJECT_ARG_TYPE_NAME,AML_OBJECT_ARG_TYPE_UINT8),
// 	DECL_OPCODE(AML_OPCODE_LOCAL0,0),
// 	DECL_OPCODE(AML_OPCODE_LOCAL1,0),
// 	DECL_OPCODE(AML_OPCODE_LOCAL2,0),
// 	DECL_OPCODE(AML_OPCODE_LOCAL3,0),
// 	DECL_OPCODE(AML_OPCODE_LOCAL4,0),
// 	DECL_OPCODE(AML_OPCODE_LOCAL5,0),
// 	DECL_OPCODE(AML_OPCODE_LOCAL6,0),
// 	DECL_OPCODE(AML_OPCODE_LOCAL7,0),
// 	DECL_OPCODE(AML_OPCODE_ARG0,0),
// 	DECL_OPCODE(AML_OPCODE_ARG1,0),
// 	DECL_OPCODE(AML_OPCODE_ARG2,0),
// 	DECL_OPCODE(AML_OPCODE_ARG3,0),
// 	DECL_OPCODE(AML_OPCODE_ARG4,0),
// 	DECL_OPCODE(AML_OPCODE_ARG5,0),
// 	DECL_OPCODE(AML_OPCODE_ARG6,0),
// 	DECL_OPCODE(AML_OPCODE_STORE,0,AML_OBJECT_ARG_TYPE_OBJECT,AML_OBJECT_ARG_TYPE_OBJECT),
// 	DECL_OPCODE(AML_OPCODE_REF_OF,0,AML_OBJECT_ARG_TYPE_OBJECT),
// 	DECL_OPCODE(AML_OPCODE_ADD,0,AML_OBJECT_ARG_TYPE_OBJECT,AML_OBJECT_ARG_TYPE_OBJECT,AML_OBJECT_ARG_TYPE_OBJECT),
// 	DECL_OPCODE(AML_OPCODE_CONCAT,0,AML_OBJECT_ARG_TYPE_OBJECT,AML_OBJECT_ARG_TYPE_OBJECT,AML_OBJECT_ARG_TYPE_OBJECT),
// 	DECL_OPCODE(AML_OPCODE_SUBTRACT,0,AML_OBJECT_ARG_TYPE_OBJECT,AML_OBJECT_ARG_TYPE_OBJECT,AML_OBJECT_ARG_TYPE_OBJECT),
// 	DECL_OPCODE(AML_OPCODE_INCREMENT,0,AML_OBJECT_ARG_TYPE_OBJECT),
// 	DECL_OPCODE(AML_OPCODE_DECREMENT,0,AML_OBJECT_ARG_TYPE_OBJECT),
// 	DECL_OPCODE(AML_OPCODE_MULTIPLY,0,AML_OBJECT_ARG_TYPE_OBJECT,AML_OBJECT_ARG_TYPE_OBJECT,AML_OBJECT_ARG_TYPE_OBJECT),
// 	DECL_OPCODE(AML_OPCODE_DIVIDE,0,AML_OBJECT_ARG_TYPE_OBJECT,AML_OBJECT_ARG_TYPE_OBJECT,AML_OBJECT_ARG_TYPE_OBJECT,AML_OBJECT_ARG_TYPE_OBJECT),
// 	DECL_OPCODE(AML_OPCODE_SHIFT_LEFT,0,AML_OBJECT_ARG_TYPE_OBJECT,AML_OBJECT_ARG_TYPE_OBJECT,AML_OBJECT_ARG_TYPE_OBJECT),
// 	DECL_OPCODE(AML_OPCODE_SHIFT_RIGHT,0,AML_OBJECT_ARG_TYPE_OBJECT,AML_OBJECT_ARG_TYPE_OBJECT,AML_OBJECT_ARG_TYPE_OBJECT),
// 	DECL_OPCODE(AML_OPCODE_AND,0,AML_OBJECT_ARG_TYPE_OBJECT,AML_OBJECT_ARG_TYPE_OBJECT,AML_OBJECT_ARG_TYPE_OBJECT),
// 	DECL_OPCODE(AML_OPCODE_NAND,0,AML_OBJECT_ARG_TYPE_OBJECT,AML_OBJECT_ARG_TYPE_OBJECT,AML_OBJECT_ARG_TYPE_OBJECT),
// 	DECL_OPCODE(AML_OPCODE_OR,0,AML_OBJECT_ARG_TYPE_OBJECT,AML_OBJECT_ARG_TYPE_OBJECT,AML_OBJECT_ARG_TYPE_OBJECT),
// 	DECL_OPCODE(AML_OPCODE_NOR,0,AML_OBJECT_ARG_TYPE_OBJECT,AML_OBJECT_ARG_TYPE_OBJECT,AML_OBJECT_ARG_TYPE_OBJECT),
// 	DECL_OPCODE(AML_OPCODE_XOR,0,AML_OBJECT_ARG_TYPE_OBJECT,AML_OBJECT_ARG_TYPE_OBJECT,AML_OBJECT_ARG_TYPE_OBJECT),
// 	DECL_OPCODE(AML_OPCODE_NOT,0,AML_OBJECT_ARG_TYPE_OBJECT,AML_OBJECT_ARG_TYPE_OBJECT),
// 	DECL_OPCODE(AML_OPCODE_FIND_SET_LEFT_BIT,0,AML_OBJECT_ARG_TYPE_OBJECT,AML_OBJECT_ARG_TYPE_OBJECT),
// 	DECL_OPCODE(AML_OPCODE_FIND_SET_RIGHT_BIT,0,AML_OBJECT_ARG_TYPE_OBJECT,AML_OBJECT_ARG_TYPE_OBJECT),
// 	DECL_OPCODE(AML_OPCODE_DEREF_OF,0,AML_OBJECT_ARG_TYPE_OBJECT),
// 	DECL_OPCODE(AML_OPCODE_CONCAT_RES,0,AML_OBJECT_ARG_TYPE_OBJECT,AML_OBJECT_ARG_TYPE_OBJECT,AML_OBJECT_ARG_TYPE_OBJECT),
// 	DECL_OPCODE(AML_OPCODE_MOD,0,AML_OBJECT_ARG_TYPE_OBJECT,AML_OBJECT_ARG_TYPE_OBJECT,AML_OBJECT_ARG_TYPE_OBJECT),
// 	DECL_OPCODE(AML_OPCODE_NOTIFY,0,AML_OBJECT_ARG_TYPE_OBJECT,AML_OBJECT_ARG_TYPE_OBJECT),
// 	DECL_OPCODE(AML_OPCODE_SIZE_OF,0,AML_OBJECT_ARG_TYPE_OBJECT),
// 	DECL_OPCODE(AML_OPCODE_INDEX,0,AML_OBJECT_ARG_TYPE_OBJECT,AML_OBJECT_ARG_TYPE_OBJECT,AML_OBJECT_ARG_TYPE_OBJECT),
// 	DECL_OPCODE(AML_OPCODE_MATCH,0,AML_OBJECT_ARG_TYPE_OBJECT,AML_OBJECT_ARG_TYPE_UINT8,AML_OBJECT_ARG_TYPE_OBJECT,AML_OBJECT_ARG_TYPE_UINT8,AML_OBJECT_ARG_TYPE_OBJECT,AML_OBJECT_ARG_TYPE_OBJECT),
// 	DECL_OPCODE(AML_OPCODE_CREATE_DWORD_FIELD,0,AML_OBJECT_ARG_TYPE_OBJECT,AML_OBJECT_ARG_TYPE_OBJECT,AML_OBJECT_ARG_TYPE_NAME),
// 	DECL_OPCODE(AML_OPCODE_CREATE_WORD_FIELD,0,AML_OBJECT_ARG_TYPE_OBJECT,AML_OBJECT_ARG_TYPE_OBJECT,AML_OBJECT_ARG_TYPE_NAME),
// 	DECL_OPCODE(AML_OPCODE_CREATE_BYTE_FIELD,0,AML_OBJECT_ARG_TYPE_OBJECT,AML_OBJECT_ARG_TYPE_OBJECT,AML_OBJECT_ARG_TYPE_NAME),
// 	DECL_OPCODE(AML_OPCODE_CREATE_BIT_FIELD,0,AML_OBJECT_ARG_TYPE_OBJECT,AML_OBJECT_ARG_TYPE_OBJECT,AML_OBJECT_ARG_TYPE_NAME),
// 	DECL_OPCODE(AML_OPCODE_OBJECT_TYPE,0,AML_OBJECT_ARG_TYPE_OBJECT),
// 	DECL_OPCODE(AML_OPCODE_CREATE_QWORD_FIELD,0,AML_OBJECT_ARG_TYPE_OBJECT,AML_OBJECT_ARG_TYPE_OBJECT,AML_OBJECT_ARG_TYPE_NAME),
// 	DECL_OPCODE(AML_OPCODE_L_AND,0,AML_OBJECT_ARG_TYPE_OBJECT,AML_OBJECT_ARG_TYPE_OBJECT),
// 	DECL_OPCODE(AML_OPCODE_L_OR,0,AML_OBJECT_ARG_TYPE_OBJECT,AML_OBJECT_ARG_TYPE_OBJECT),
// 	DECL_OPCODE(AML_OPCODE_L_NOT,0,AML_OBJECT_ARG_TYPE_OBJECT),
// 	DECL_OPCODE(AML_OPCODE_L_EQUAL,0,AML_OBJECT_ARG_TYPE_OBJECT,AML_OBJECT_ARG_TYPE_OBJECT),
// 	DECL_OPCODE(AML_OPCODE_L_GREATER,0,AML_OBJECT_ARG_TYPE_OBJECT,AML_OBJECT_ARG_TYPE_OBJECT),
// 	DECL_OPCODE(AML_OPCODE_L_LESS,0,AML_OBJECT_ARG_TYPE_OBJECT,AML_OBJECT_ARG_TYPE_OBJECT),
// 	DECL_OPCODE(AML_OPCODE_TO_BUFFER,0,AML_OBJECT_ARG_TYPE_OBJECT,AML_OBJECT_ARG_TYPE_OBJECT),
// 	DECL_OPCODE(AML_OPCODE_TO_DECIMAL_STRING,0,AML_OBJECT_ARG_TYPE_OBJECT,AML_OBJECT_ARG_TYPE_OBJECT),
// 	DECL_OPCODE(AML_OPCODE_TO_HEX_STRING,0,AML_OBJECT_ARG_TYPE_OBJECT,AML_OBJECT_ARG_TYPE_OBJECT),
// 	DECL_OPCODE(AML_OPCODE_TO_INTEGER,0,AML_OBJECT_ARG_TYPE_OBJECT,AML_OBJECT_ARG_TYPE_OBJECT),
// 	DECL_OPCODE(AML_OPCODE_TO_STRING,0,AML_OBJECT_ARG_TYPE_OBJECT,AML_OBJECT_ARG_TYPE_OBJECT,AML_OBJECT_ARG_TYPE_OBJECT),
// 	DECL_OPCODE(AML_OPCODE_COPY_OBJECT,0,AML_OBJECT_ARG_TYPE_OBJECT,AML_OBJECT_ARG_TYPE_OBJECT),
// 	DECL_OPCODE(AML_OPCODE_MID,0,AML_OBJECT_ARG_TYPE_OBJECT,AML_OBJECT_ARG_TYPE_OBJECT,AML_OBJECT_ARG_TYPE_OBJECT),
// 	DECL_OPCODE(AML_OPCODE_CONTINUE,0),
// 	DECL_OPCODE(AML_OPCODE_IF,OPCODE_FLAG_PKGLENGTH,AML_OBJECT_ARG_TYPE_OBJECT),
// 	DECL_OPCODE(AML_OPCODE_ELSE,OPCODE_FLAG_PKGLENGTH),
// 	DECL_OPCODE(AML_OPCODE_WHILE,OPCODE_FLAG_PKGLENGTH,AML_OBJECT_ARG_TYPE_OBJECT),
// 	DECL_OPCODE(AML_OPCODE_NOOP,0),
// 	DECL_OPCODE(AML_OPCODE_RETURN,0,AML_OBJECT_ARG_TYPE_OBJECT),
// 	DECL_OPCODE(AML_OPCODE_BREAK,0),
// 	DECL_OPCODE(AML_OPCODE_BREAK_POINT,0),
// 	DECL_OPCODE(AML_OPCODE_ONES,0),
// 	DECL_OPCODE(AML_OPCODE_EXT_MUTEX,OPCODE_FLAG_EXTENDED,AML_OBJECT_ARG_TYPE_NAME,AML_OBJECT_ARG_TYPE_UINT8),
// 	DECL_OPCODE(AML_OPCODE_EXT_EVENT,OPCODE_FLAG_EXTENDED,AML_OBJECT_ARG_TYPE_NAME),
// 	DECL_OPCODE(AML_OPCODE_EXT_COND_REF_OF,OPCODE_FLAG_EXTENDED,AML_OBJECT_ARG_TYPE_OBJECT,AML_OBJECT_ARG_TYPE_OBJECT),
// 	DECL_OPCODE(AML_OPCODE_EXT_CREATE_FIELD,OPCODE_FLAG_EXTENDED,AML_OBJECT_ARG_TYPE_OBJECT,AML_OBJECT_ARG_TYPE_OBJECT,AML_OBJECT_ARG_TYPE_OBJECT,AML_OBJECT_ARG_TYPE_NAME),
// 	DECL_OPCODE(AML_OPCODE_EXT_LOAD_TABLE,OPCODE_FLAG_EXTENDED,AML_OBJECT_ARG_TYPE_OBJECT,AML_OBJECT_ARG_TYPE_OBJECT,AML_OBJECT_ARG_TYPE_OBJECT,AML_OBJECT_ARG_TYPE_OBJECT,AML_OBJECT_ARG_TYPE_OBJECT,AML_OBJECT_ARG_TYPE_OBJECT),
// 	DECL_OPCODE(AML_OPCODE_EXT_LOAD,OPCODE_FLAG_EXTENDED,AML_OBJECT_ARG_TYPE_NAME,AML_OBJECT_ARG_TYPE_OBJECT),
// 	DECL_OPCODE(AML_OPCODE_EXT_STALL,OPCODE_FLAG_EXTENDED,AML_OBJECT_ARG_TYPE_OBJECT),
// 	DECL_OPCODE(AML_OPCODE_EXT_SLEEP,OPCODE_FLAG_EXTENDED,AML_OBJECT_ARG_TYPE_OBJECT),
// 	DECL_OPCODE(AML_OPCODE_EXT_ACQUIRE,OPCODE_FLAG_EXTENDED,AML_OBJECT_ARG_TYPE_OBJECT,AML_OBJECT_ARG_TYPE_UINT16),
// 	DECL_OPCODE(AML_OPCODE_EXT_SIGNAL,OPCODE_FLAG_EXTENDED,AML_OBJECT_ARG_TYPE_OBJECT),
// 	DECL_OPCODE(AML_OPCODE_EXT_WAIT,OPCODE_FLAG_EXTENDED,AML_OBJECT_ARG_TYPE_OBJECT,AML_OBJECT_ARG_TYPE_OBJECT),
// 	DECL_OPCODE(AML_OPCODE_EXT_RESET,OPCODE_FLAG_EXTENDED,AML_OBJECT_ARG_TYPE_OBJECT),
// 	DECL_OPCODE(AML_OPCODE_EXT_RELEASE,OPCODE_FLAG_EXTENDED,AML_OBJECT_ARG_TYPE_OBJECT),
// 	DECL_OPCODE(AML_OPCODE_EXT_FROM_BCD,OPCODE_FLAG_EXTENDED,AML_OBJECT_ARG_TYPE_OBJECT,AML_OBJECT_ARG_TYPE_OBJECT),
// 	DECL_OPCODE(AML_OPCODE_EXT_TO_BCD,OPCODE_FLAG_EXTENDED,AML_OBJECT_ARG_TYPE_OBJECT,AML_OBJECT_ARG_TYPE_OBJECT),
// 	DECL_OPCODE(AML_OPCODE_EXT_UNLOAD,OPCODE_FLAG_EXTENDED,AML_OBJECT_ARG_TYPE_OBJECT),
// 	DECL_OPCODE(AML_OPCODE_EXT_REVISION,OPCODE_FLAG_EXTENDED),
// 	DECL_OPCODE(AML_OPCODE_EXT_DEBUG,OPCODE_FLAG_EXTENDED),
// 	DECL_OPCODE(AML_OPCODE_EXT_FATAL,OPCODE_FLAG_EXTENDED,AML_OBJECT_ARG_TYPE_UINT8,AML_OBJECT_ARG_TYPE_UINT32,AML_OBJECT_ARG_TYPE_OBJECT),
// 	DECL_OPCODE(AML_OPCODE_EXT_TIMER,OPCODE_FLAG_EXTENDED),
// 	DECL_OPCODE(AML_OPCODE_EXT_REGION,OPCODE_FLAG_EXTENDED,AML_OBJECT_ARG_TYPE_NAME,AML_OBJECT_ARG_TYPE_UINT8,AML_OBJECT_ARG_TYPE_OBJECT,AML_OBJECT_ARG_TYPE_OBJECT),
// 	DECL_OPCODE(AML_OPCODE_EXT_FIELD,OPCODE_FLAG_EXTENDED|OPCODE_FLAG_PKGLENGTH|OPCODE_FLAG_EXTRA_BYTES,AML_OBJECT_ARG_TYPE_NAME,AML_OBJECT_ARG_TYPE_UINT8),
// 	DECL_OPCODE(AML_OPCODE_EXT_DEVICE,OPCODE_FLAG_EXTENDED|OPCODE_FLAG_PKGLENGTH,AML_OBJECT_ARG_TYPE_NAME),
// 	DECL_OPCODE(AML_OPCODE_EXT_PROCESSOR,OPCODE_FLAG_EXTENDED|OPCODE_FLAG_PKGLENGTH,AML_OBJECT_ARG_TYPE_NAME,AML_OBJECT_ARG_TYPE_UINT8,AML_OBJECT_ARG_TYPE_UINT32,AML_OBJECT_ARG_TYPE_UINT8),
// 	DECL_OPCODE(AML_OPCODE_EXT_POWER_RES,OPCODE_FLAG_EXTENDED|OPCODE_FLAG_PKGLENGTH,AML_OBJECT_ARG_TYPE_NAME,AML_OBJECT_ARG_TYPE_UINT8,AML_OBJECT_ARG_TYPE_UINT16),
// 	DECL_OPCODE(AML_OPCODE_EXT_THERMAL_ZONE,OPCODE_FLAG_EXTENDED|OPCODE_FLAG_PKGLENGTH,AML_OBJECT_ARG_TYPE_NAME),
// 	DECL_OPCODE(AML_OPCODE_EXT_INDEX_FIELD,OPCODE_FLAG_EXTENDED|OPCODE_FLAG_PKGLENGTH|OPCODE_FLAG_EXTRA_BYTES,AML_OBJECT_ARG_TYPE_NAME,AML_OBJECT_ARG_TYPE_NAME,AML_OBJECT_ARG_TYPE_UINT8),
// 	DECL_OPCODE(AML_OPCODE_EXT_BANK_FIELD,OPCODE_FLAG_EXTENDED|OPCODE_FLAG_PKGLENGTH|OPCODE_FLAG_EXTRA_BYTES,AML_OBJECT_ARG_TYPE_NAME,AML_OBJECT_ARG_TYPE_NAME,AML_OBJECT_ARG_TYPE_OBJECT,AML_OBJECT_ARG_TYPE_UINT8),
// 	DECL_OPCODE(AML_OPCODE_EXT_DATA_REGION,OPCODE_FLAG_EXTENDED,AML_OBJECT_ARG_TYPE_NAME,AML_OBJECT_ARG_TYPE_OBJECT,AML_OBJECT_ARG_TYPE_OBJECT,AML_OBJECT_ARG_TYPE_OBJECT),
// 	DECL_OPCODE(AML_OPCODE_NAME_REFERENCE,0)
// };



// static omm_allocator_t* _aml_object_allocator=NULL;



// static const opcode_t* _parse_opcode(const u8* data){
// 	u8 type=data[0];
// 	u8 extended=0;
// 	if (type==AML_EXTENDED_OPCODE){
// 		extended=OPCODE_FLAG_EXTENDED;
// 		type=data[1];
// 	}
// 	const opcode_t* out=_aml_opcodes;
// 	for (;out->opcode!=AML_OPCODE_NAME_REFERENCE;out++){
// 		if ((out->flags&OPCODE_FLAG_EXTENDED)==extended&&out->opcode==type){
// 			return out;
// 		}
// 	}
// 	if (!extended&&(type=='\\'||type=='^'||type=='.'||type=='/'||(type>47&&type<58)||type=='_'||(type>64&&type<91))){
// 		return out;
// 	}
// 	ERROR("Unknown AML opcode '%s%x'",(extended?"5b ":""),type);
// 	panic("Unknown AML opcode");
// 	return NULL;
// }



// static u32 _get_opcode_encoding_length(const opcode_t* opcode){
// 	return ((opcode->flags&OPCODE_FLAG_EXTENDED)?2:1);
// }



// static u32 _get_pkglength_encoding_length(const u8* data){
// 	return (data[0]>>6)+1;
// }



// static u32 _get_name_encoding_length(const u8* data){
// 	u32 out=0;
// 	if (data[out]=='\\'){
// 		out++;
// 	}
// 	while (data[out]=='^'){
// 		out++;
// 	}
// 	u8 segment_count=1;
// 	if (data[out]=='.'){
// 		out++;
// 		segment_count=2;
// 	}
// 	else if (data[out]=='/'){
// 		out++;
// 		segment_count=data[out];
// 		out++;
// 	}
// 	else if (!data[out]){
// 		out++;
// 		segment_count=0;
// 	}
// 	return out+(segment_count<<2);
// }



// static u32 _get_pkglength(const u8* data){
// 	u32 out=data[0]&0x3f;
// 	for (u8 i=0;i<(data[0]>>6);i++){
// 		out|=data[i+1]<<(4+8*i);
// 	}
// 	return out;
// }



// static string_t* _get_decoded_name(const u8* data){
// 	char buffer[4096];
// 	u16 length=0;
// 	u32 index=0;
// 	while (data[index]=='\\'||data[index]=='^'){
// 		buffer[length]=data[index];
// 		length++;
// 		index++;
// 	}
// 	u8 segment_count=1;
// 	if (data[index]=='.'){
// 		index++;
// 		segment_count=2;
// 	}
// 	else if (data[index]=='/'){
// 		index++;
// 		segment_count=data[index];
// 		index++;
// 	}
// 	else if (!data[index]){
// 		index++;
// 		segment_count=0;
// 	}
// 	while (segment_count){
// 		segment_count--;
// 		for (u8 i=0;i<4;i++){
// 			buffer[length]=data[index];
// 			length++;
// 			index++;
// 		}
// 		if (segment_count){
// 			buffer[length]='.';
// 			length++;
// 		}
// 	}
// 	return smm_alloc(buffer,length);
// }



// static u32 _get_full_opcode_length(const u8* data,const opcode_t* opcode){
// 	if (opcode->opcode==AML_OPCODE_NAME_REFERENCE){
// 		return _get_name_encoding_length(data);
// 	}
// 	u32 out=_get_opcode_encoding_length(opcode);
// 	if (opcode->flags&OPCODE_FLAG_PKGLENGTH){
// 		return out+_get_pkglength(data+out);
// 	}
// 	for (u8 i=0;i<opcode->arg_count;i++){
// 		if (opcode->args[i]==AML_OBJECT_ARG_TYPE_UINT8){
// 			out++;
// 		}
// 		else if (opcode->args[i]==AML_OBJECT_ARG_TYPE_UINT16){
// 			out+=2;
// 		}
// 		else if (opcode->args[i]==AML_OBJECT_ARG_TYPE_UINT32){
// 			out+=4;
// 		}
// 		else if (opcode->args[i]==AML_OBJECT_ARG_TYPE_UINT64){
// 			out+=8;
// 		}
// 		else if (opcode->args[i]==AML_OBJECT_ARG_TYPE_STRING){
// 			do{
// 				out++;
// 			} while (data[out-1]);
// 		}
// 		else if (opcode->args[i]==AML_OBJECT_ARG_TYPE_NAME){
// 			out+=_get_name_encoding_length(data+out);
// 		}
// 		else if (opcode->args[i]==AML_OBJECT_ARG_TYPE_OBJECT){
// 			out+=_get_full_opcode_length(data+out,_parse_opcode(data+out));
// 		}
// 	}
// 	return out;
// }



// static u32 _parse_object(const u8* data,aml_object_t* out){
// 	const u8* data_start=data;
// 	const u8* data_end=NULL;
// 	const opcode_t* opcode=_parse_opcode(data);
// 	if (opcode->flags&OPCODE_FLAG_EXTENDED){
// 		out->opcode[0]=AML_EXTENDED_OPCODE;
// 		out->opcode[1]=opcode->opcode;
// 	}
// 	else{
// 		out->opcode[0]=opcode->opcode;
// 		out->opcode[1]=0;
// 	}
// 	if (opcode->opcode==AML_OPCODE_NAME_REFERENCE){
// 		out->arg_count=1;
// 		out->flags=0;
// 		out->args[0].string=_get_decoded_name(data);
// 		out->next=NULL;
// 		return _get_name_encoding_length(data);
// 	}
// 	data+=_get_opcode_encoding_length(opcode);
// 	if (opcode->flags&OPCODE_FLAG_PKGLENGTH){
// 		data_end=data+_get_pkglength(data);
// 		data+=_get_pkglength_encoding_length(data);
// 	}
// 	out->arg_count=opcode->arg_count;
// 	out->flags=((opcode->flags&OPCODE_FLAG_EXTRA_BYTES)?AML_OBJECT_FLAG_BYTE_DATA:0);
// 	out->next=NULL;
// 	for (u8 i=0;i<opcode->arg_count;i++){
// 		out->args[i].type=opcode->args[i];
// 		if (opcode->args[i]==AML_OBJECT_ARG_TYPE_UINT8){
// 			out->args[i].number=data[0];
// 			data++;
// 		}
// 		else if (opcode->args[i]==AML_OBJECT_ARG_TYPE_UINT16){
// 			out->args[i].number=(data[0]<<8)|data[1];
// 			data+=2;
// 		}
// 		else if (opcode->args[i]==AML_OBJECT_ARG_TYPE_UINT32){
// 			out->args[i].number=(data[0]<<24)|(data[1]<<16)|(data[2]<<8)|data[3];
// 			data+=4;
// 		}
// 		else if (opcode->args[i]==AML_OBJECT_ARG_TYPE_UINT64){
// 			out->args[i].number=(((u64)(data[0]))<<56)|(((u64)(data[1]))<<48)|(((u64)(data[2]))<<40)|(((u64)(data[3]))<<32)|(((u64)(data[4]))<<24)|(((u64)(data[5]))<<16)|(((u64)(data[6]))<<8)|((u64)(data[7]));
// 			data+=8;
// 		}
// 		else if (opcode->args[i]==AML_OBJECT_ARG_TYPE_STRING){
// 			out->args[i].string=smm_alloc((const char*)data,0);
// 			data+=out->args[i].string->length+1;
// 		}
// 		else if (opcode->args[i]==AML_OBJECT_ARG_TYPE_NAME){
// 			out->args[i].string=_get_decoded_name(data);
// 			data+=_get_name_encoding_length(data);
// 		}
// 		else if (opcode->args[i]==AML_OBJECT_ARG_TYPE_OBJECT){
// 			out->args[i].object=omm_alloc(_aml_object_allocator);
// 			data+=_parse_object(data,out->args[i].object);
// 		}
// 	}
// 	if (!data_end){
// 		return data-data_start;
// 	}
// 	if (opcode->flags&OPCODE_FLAG_EXTRA_BYTES){
// 		out->data.bytes=data;
// 		out->data_length=data_end-data;
// 		return data_end-data_start;
// 	}
// 	out->data_length=0;
// 	for (u32 i=0;data+i<data_end;){
// 		out->data_length++;
// 		i+=_get_full_opcode_length(data+i,_parse_opcode(data+i));
// 	}
// 	out->data.child=NULL;
// 	aml_object_t* last_child=NULL;
// 	for (u32 i=0;i<out->data_length;i++){
// 		aml_object_t* child=omm_alloc(_aml_object_allocator);
// 		data+=_parse_object(data,child);
// 		if (last_child){
// 			last_child->next=child;
// 		}
// 		else{
// 			out->data.child=child;
// 		}
// 		last_child=child;
// 	}
// 	return data_end-data_start;
// }



// aml_object_t* aml_parse(const u8* data,u32 length){
// 	LOG("Loading AML...");
// 	INFO("Found AML code at %p (%v)",data,length);
// 	_aml_object_allocator=omm_init("aml_object",sizeof(aml_object_t),8,2,pmm_alloc_counter("omm_aml_object"));
// 	spinlock_init(&(_aml_object_allocator->lock));
// 	aml_object_t* root=omm_alloc(_aml_object_allocator);
// 	root->opcode[0]=AML_OPCODE_ROOT;
// 	root->opcode[1]=0;
// 	root->arg_count=0;
// 	root->flags=0;
// 	root->data_length=0;
// 	root->next=NULL;
// 	for (u32 offset=0;offset<length;){
// 		root->data_length++;
// 		offset+=_get_full_opcode_length(data+offset,_parse_opcode(data+offset));
// 	}
// 	root->data.child=NULL;
// 	aml_object_t* last_child=NULL;
// 	u32 offset=0;
// 	for (u32 i=0;i<root->data_length;i++){
// 		aml_object_t* child=omm_alloc(_aml_object_allocator);
// 		offset+=_parse_object(data+offset,child);
// 		if (i){
// 			last_child->next=child;
// 		}
// 		else{
// 			root->data.child=child;
// 		}
// 		last_child=child;
// 	}
// 	return root;
// }



// u32 aml_parse_pkglength(const u8* data,u32* out){
// 	*out=_get_pkglength(data);
// 	return _get_pkglength_encoding_length(data);
// }
