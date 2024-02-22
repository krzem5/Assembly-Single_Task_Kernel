#include <glsl/_internal/error.h>
#include <glsl/_internal/lexer.h>
#include <glsl/error.h>
#include <glsl/preprocessor.h>
#include <glsl/version.h>
#include <sys/heap/heap.h>
#include <sys/memory/memory.h>
#include <sys/string/string.h>
#include <sys/types.h>



#define STATE_BUFFER_GROWTH_SIZE 256 // power of 2



static void _emit_string(glsl_preprocessor_state_t* state,const char* buffer,u32 length){
	if (!length){
		length=sys_string_length(buffer);
	}
	if (state->_capacity-state->length<length){
		state->_capacity=(state->length+length+STATE_BUFFER_GROWTH_SIZE)&(-STATE_BUFFER_GROWTH_SIZE);
		state->data=sys_heap_realloc(NULL,state->data,state->_capacity);
	}
	sys_memory_copy(buffer,state->data+state->length,length);
	state->length+=length;
	state->data[state->length]=0;
}



static void _emit_char(glsl_preprocessor_state_t* state,char c){
	_emit_string(state,&c,1);
}



static void _emit_line(glsl_preprocessor_state_t* state,u32 line,u32 file){
	state->line_count++;
	state->lines=sys_heap_realloc(NULL,state->lines,state->line_count*sizeof(glsl_preprocessor_line_t));
	(state->lines+state->line_count-1)->offset=state->length;
	(state->lines+state->line_count-1)->line=line;
	(state->lines+state->line_count-1)->file=file;
}



static _Bool _begins_with_word(const char** src,const char* word){
	u32 length=sys_string_length(word);
	if (!sys_memory_compare(*src,word,length)&&(!(*src)[length]||LEXER_IS_WHITESPACE((*src)[length]))){
		(*src)+=length;
		return 1;
	}
	return 0;
}



static glsl_error_t _parse_int(const char** src,u64* out){
	const char* ptr=*src;
	u64 value=0;
	if (ptr[0]!='0'){
		do{
			if (!LEXER_IS_DIGIT(ptr[0])){
				return _glsl_error_create_lexer_digit_expected(ptr[0],10);
			}
			value=value*10+ptr[0]-48;
			ptr++;
		} while (LEXER_IS_IDENTIFIER(ptr[0]));
		goto _no_error;
	}
	ptr++;
	if (ptr[0]!='x'&&ptr[0]!='X'){
		do{
			if (!LEXER_IS_OCT_DIGIT(ptr[0])){
				return _glsl_error_create_lexer_digit_expected(ptr[0],8);
			}
			value=(value<<3)+ptr[0]-48;
			ptr++;
		} while (LEXER_IS_IDENTIFIER(ptr[0]));
		goto _no_error;
	}
	ptr++;
	do{
		if (!LEXER_IS_HEX_DIGIT(ptr[0])){
			return _glsl_error_create_lexer_digit_expected(ptr[0],16);
		}
		value=(value<<4)+ptr[0]-(LEXER_IS_DIGIT(ptr[0])?48:55+((ptr[0]>='a')<<5));
		ptr++;
	} while (LEXER_IS_IDENTIFIER(ptr[0]));
_no_error:
	*src=ptr;
	*out=value;
	return GLSL_NO_ERROR;
}



static const char* _find_macro(glsl_preprocessor_state_t* state,const char* identifier,u32 length){
	if (!length){
		length=sys_string_length(identifier);
	}
	for (u32 i=0;i<state->macro_count;i++){
		if ((state->macros+i)->identifier_length==length&&!sys_memory_compare((state->macros+i)->identifier,identifier,length)){
			return (state->macros+i)->value;
		}
	}
	return NULL;
}



static void _create_macro(glsl_preprocessor_state_t* state,char* identifier,char* value){
	state->macro_count++;
	state->macros=sys_heap_realloc(NULL,state->macros,state->macro_count*sizeof(glsl_preprocessor_macro_t));
	(state->macros+state->macro_count-1)->identifier=identifier;
	(state->macros+state->macro_count-1)->identifier_length=sys_string_length(identifier);
	(state->macros+state->macro_count-1)->value=value;
}



static void _delete_macro(glsl_preprocessor_state_t* state,char* identifier){
	u32 length=sys_string_length(identifier);
	for (u32 i=0;i<state->macro_count;i++){
		if ((state->macros+i)->identifier_length==length&&!sys_memory_compare((state->macros+i)->identifier,identifier,length)){
			state->macro_count--;
			sys_heap_dealloc(NULL,(state->macros+i)->identifier);
			sys_heap_dealloc(NULL,(state->macros+i)->value);
			*(state->macros+i)=*(state->macros+state->macro_count);
			state->macros=sys_heap_realloc(NULL,state->macros,state->macro_count*sizeof(glsl_preprocessor_macro_t));
			return;
		}
	}
}



SYS_PUBLIC void glsl_preprocessor_state_init(glsl_preprocessor_state_t* state){
	state->data=0;
	state->length=0;
	state->_capacity=0;
	state->lines=NULL;
	state->macros=NULL;
	state->line_count=0;
	state->macro_count=0;
}



SYS_PUBLIC void glsl_preprocessor_state_deinit(glsl_preprocessor_state_t* state){
	sys_heap_dealloc(NULL,state->data);
	sys_heap_dealloc(NULL,state->lines);
	for (u32 i=0;i<state->macro_count;i++){
		sys_heap_dealloc(NULL,(state->macros+i)->identifier);
		sys_heap_dealloc(NULL,(state->macros+i)->value);
	}
	sys_heap_dealloc(NULL,state->macros);
	state->data=NULL;
	state->length=0;
	state->_capacity=0;
	state->lines=NULL;
	state->macros=0;
	state->line_count=0;
	state->macro_count=0;
}



SYS_PUBLIC glsl_error_t glsl_preprocessor_add_file(const char* src,u32 index,glsl_preprocessor_state_t* state){
	_emit_line(state,0,index);
	_Bool is_first=1;
	while (1){
		const char* line_start=src;
		for (;LEXER_IS_WHITESPACE(src[0]);src++);
		if (!src[0]){
			break;
		}
		if (src[0]=='#'){
			for (src++;src[0]!='\n'&&LEXER_IS_WHITESPACE(src[0]);src++);
			if (!src[0]||src[0]=='\n'){
				goto _emit_missing_newlines;
			}
			if (_begins_with_word(&src,"version")){
				if (!is_first){
					return _glsl_error_create_preprocessor_wrong_version_placement();
				}
				for (;src[0]!='\n'&&LEXER_IS_WHITESPACE(src[0]);src++);
				if (!LEXER_IS_DIGIT(src[0])){
					return _glsl_error_create_parser_expected("version");
				}
				u64 version=0;
				glsl_error_t error=_parse_int(&src,&version);
				if (error!=GLSL_NO_ERROR){
					return error;
				}
				if (version!=glsl_get_version()){
					return _glsl_error_create_preprocessor_invalid_version(version);
				}
				for (;src[0]!='\n'&&LEXER_IS_WHITESPACE(src[0]);src++);
				if (src[0]!='\n'){
					if (!_begins_with_word(&src,"core")){
						u32 length=0;
						for (;LEXER_IS_IDENTIFIER(src[length]);length++);
						return _glsl_error_create_preprocessor_invalid_profile(src,length);
					}
				}
			}
			else if (_begins_with_word(&src,"define")){
				for (;src[0]!='\n'&&LEXER_IS_WHITESPACE(src[0]);src++);
				if (!LEXER_IS_IDENTIFIER_START(src[0])){
					return _glsl_error_create_parser_expected("macro name");
				}
				u32 length=0;
				for (;LEXER_IS_IDENTIFIER(src[length]);length++);
				char* identifier=sys_heap_alloc(NULL,length+1);
				sys_memory_copy(src,identifier,length);
				identifier[length]=0;
				src+=length;
				for (;src[0]!='\n'&&LEXER_IS_WHITESPACE(src[0]);src++);
				if (src[0]=='('){
					sys_heap_dealloc(NULL,identifier);
					return _glsl_error_create_unimplemented(__FILE__,__LINE__,__func__);
				}
				if (_find_macro(state,identifier,0)){
					glsl_error_t error=_glsl_error_create_preprocessor_already_defined(identifier);
					sys_heap_dealloc(NULL,identifier);
					return error;
				}
				for (length=0;src[length]&&src[length]!='\n';length++);
				char* value=sys_heap_alloc(NULL,length+1);
				sys_memory_copy(src,value,length);
				value[length]=0;
				src+=length;
				_create_macro(state,identifier,value);
			}
			else if (_begins_with_word(&src,"undef")){
				for (;src[0]!='\n'&&LEXER_IS_WHITESPACE(src[0]);src++);
				if (!LEXER_IS_IDENTIFIER_START(src[0])){
					return _glsl_error_create_parser_expected("macro name");
				}
				u32 length=0;
				for (;LEXER_IS_IDENTIFIER(src[length]);length++);
				char* identifier=sys_heap_alloc(NULL,length+1);
				sys_memory_copy(src,identifier,length);
				identifier[length]=0;
				src+=length;
				_delete_macro(state,identifier);
				sys_heap_dealloc(NULL,identifier);
			}
			else if (_begins_with_word(&src,"if")){
				return _glsl_error_create_unimplemented(__FILE__,__LINE__,__func__);
			}
			else if (_begins_with_word(&src,"ifdef")){
				return _glsl_error_create_unimplemented(__FILE__,__LINE__,__func__);
			}
			else if (_begins_with_word(&src,"ifndef")){
				return _glsl_error_create_unimplemented(__FILE__,__LINE__,__func__);
			}
			else if (_begins_with_word(&src,"else")){
				return _glsl_error_create_unimplemented(__FILE__,__LINE__,__func__);
			}
			else if (_begins_with_word(&src,"elif")){
				return _glsl_error_create_unimplemented(__FILE__,__LINE__,__func__);
			}
			else if (_begins_with_word(&src,"endif")){
				return _glsl_error_create_unimplemented(__FILE__,__LINE__,__func__);
			}
			else if (_begins_with_word(&src,"error")){
				return _glsl_error_create_unimplemented(__FILE__,__LINE__,__func__);
			}
			else if (_begins_with_word(&src,"pragma")){
				return _glsl_error_create_unimplemented(__FILE__,__LINE__,__func__);
			}
			else if (_begins_with_word(&src,"extension")){
				return _glsl_error_create_unimplemented(__FILE__,__LINE__,__func__);
			}
			else if (_begins_with_word(&src,"line")){
				for (;src[0]!='\n'&&LEXER_IS_WHITESPACE(src[0]);src++);
				if (!LEXER_IS_DIGIT(src[0])){
					return _glsl_error_create_parser_expected("line number");
				}
				u64 line_number=0;
				glsl_error_t error=_parse_int(&src,&line_number);
				if (error!=GLSL_NO_ERROR){
					return error;
				}
				for (;src[0]!='\n'&&LEXER_IS_WHITESPACE(src[0]);src++);
				u64 file_number=(state->lines+state->line_count-1)->file;
				if (src[0]!='\n'){
					if (!LEXER_IS_DIGIT(src[0])){
						return _glsl_error_create_parser_expected("file number");
					}
					glsl_error_t error=_parse_int(&src,&file_number);
					if (error!=GLSL_NO_ERROR){
						return error;
					}
				}
				_emit_line(state,line_number,file_number);
			}
			else{
				u32 length=0;
				for (;LEXER_IS_IDENTIFIER(src[length]);length++);
				return _glsl_error_create_preprocessor_unknown_directive(src,length);
			}
			is_first=0;
			for (;src[0]&&src[0]!='\n';src++);
_emit_missing_newlines:
			for (;line_start!=src;line_start++){
				if (line_start[0]=='\n'){
					_emit_char(state,'\n');
				}
			}
			continue;
		}
		while (src[0]!='\n'){
			if (LEXER_IS_IDENTIFIER_START(src[0])){
				const char* identifier=src;
				for (src++;LEXER_IS_IDENTIFIER(src[0]);src++);
				const char* macro_value=_find_macro(state,identifier,src-identifier);
				if (macro_value){
					_emit_string(state,line_start,identifier-line_start);
					_emit_string(state,macro_value,0);
					line_start=src;
				}
			}
			else{
				src++;
			}
		}
		_emit_string(state,line_start,src-line_start);
		is_first=0;
	}
	_emit_char(state,'\n');
	return GLSL_NO_ERROR;
}
