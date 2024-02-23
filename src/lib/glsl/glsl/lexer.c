#include <glsl/_internal/error.h>
#include <glsl/_internal/lexer.h>
#include <glsl/builtin_types.h>
#include <glsl/error.h>
#include <glsl/lexer.h>
#include <sys/heap/heap.h>
#include <sys/io/io.h>
#include <sys/memory/memory.h>
#include <sys/string/string.h>
#include <sys/types.h>



#define TOKEN_LIST_GROWTH_SIZE 64

#define LEXER_CHECK_TOKEN(name,value) \
	if (_consume_possible_identifier(out,&src,name,0,value)){ \
		continue; \
	}
#define LEXER_CHECK_TOKEN_WORD(name,value) \
	if (_consume_possible_identifier(out,&src,name,1,value)){ \
		continue; \
	}
#define LEXER_CHECK_TOKEN_WORD_VALUE(name,value,data) \
	token=_consume_possible_identifier(out,&src,name,1,value); \
	if (token){ \
		token->data; \
		continue; \
	}
#define LEXER_CHECK_TOKEN_WORD_RESERVED(name) \
	if (_check_possible_identifier(src,name)){ \
		error=_glsl_error_create_lexer_reserved_keyword(name); \
		goto _error; \
	}



static glsl_lexer_token_t* _emit_token(glsl_lexer_token_list_t* token_list,glsl_lexer_token_type_t glsl_lexer_token_type){
	if (token_list->length==token_list->_capacity){
		token_list->_capacity+=TOKEN_LIST_GROWTH_SIZE;
		token_list->data=sys_heap_realloc(NULL,token_list->data,token_list->_capacity*sizeof(glsl_lexer_token_t));
	}
	glsl_lexer_token_t* out=token_list->data+token_list->length;
	token_list->length++;
	out->type=glsl_lexer_token_type;
	return out;
}



static _Bool _check_possible_identifier(const char* src,const char* word){
	u32 length=sys_string_length(word);
	return (!sys_memory_compare(src,word,length)&&!LEXER_IS_IDENTIFIER(src[length]));
}



static glsl_lexer_token_t* _consume_possible_identifier(glsl_lexer_token_list_t* token_list,const char** src,const char* word,_Bool is_identifier,u32 glsl_lexer_token_type){
	u32 length=sys_string_length(word);
	if (sys_memory_compare(*src,word,length)||(is_identifier&&LEXER_IS_IDENTIFIER((*src)[length]))){
		return NULL;
	}
	(*src)+=length;
	return _emit_token(token_list,glsl_lexer_token_type);
}



SYS_PUBLIC glsl_error_t glsl_lexer_extract_tokens(const char* src,glsl_lexer_token_list_t* out){
	glsl_error_t error=GLSL_NO_ERROR;
	out->data=NULL;
	out->length=0;
	out->_capacity=0;
	while (src[0]){
		if (src[0]=='/'&&src[1]=='/'){
			for (src+=2;src[0]&&src[0]!='\n';src++);
			continue;
		}
		if (src[0]=='/'&&src[1]=='*'){
			for (src+=2;src[0]&&(src[0]!='*'||src[1]!='/');src++);
			if (src[0]=='*'){
				src+=2;
			}
			continue;
		}
		if (LEXER_IS_WHITESPACE(src[0])){
			for (src++;LEXER_IS_WHITESPACE(src[0]);src++);
			continue;
		}
		glsl_lexer_token_t* token;
		LEXER_CHECK_TOKEN_WORD_RESERVED("active");
		LEXER_CHECK_TOKEN_WORD_RESERVED("asm");
		LEXER_CHECK_TOKEN_WORD_RESERVED("cast");
		LEXER_CHECK_TOKEN_WORD_RESERVED("class");
		LEXER_CHECK_TOKEN_WORD_RESERVED("common");
		LEXER_CHECK_TOKEN_WORD_RESERVED("double");
		LEXER_CHECK_TOKEN_WORD_RESERVED("dvec2");
		LEXER_CHECK_TOKEN_WORD_RESERVED("dvec3");
		LEXER_CHECK_TOKEN_WORD_RESERVED("dvec4");
		LEXER_CHECK_TOKEN_WORD_RESERVED("enum");
		LEXER_CHECK_TOKEN_WORD_RESERVED("extern");
		LEXER_CHECK_TOKEN_WORD_RESERVED("external");
		LEXER_CHECK_TOKEN_WORD_RESERVED("filter");
		LEXER_CHECK_TOKEN_WORD_RESERVED("fixed");
		LEXER_CHECK_TOKEN_WORD_RESERVED("fvec2");
		LEXER_CHECK_TOKEN_WORD_RESERVED("fvec3");
		LEXER_CHECK_TOKEN_WORD_RESERVED("fvec4");
		LEXER_CHECK_TOKEN_WORD_RESERVED("goto");
		LEXER_CHECK_TOKEN_WORD_RESERVED("half");
		LEXER_CHECK_TOKEN_WORD_RESERVED("hvec2");
		LEXER_CHECK_TOKEN_WORD_RESERVED("hvec3");
		LEXER_CHECK_TOKEN_WORD_RESERVED("hvec4");
		LEXER_CHECK_TOKEN_WORD_RESERVED("iimage1D");
		LEXER_CHECK_TOKEN_WORD_RESERVED("iimage1DArray");
		LEXER_CHECK_TOKEN_WORD_RESERVED("iimage2D");
		LEXER_CHECK_TOKEN_WORD_RESERVED("iimage2DArray");
		LEXER_CHECK_TOKEN_WORD_RESERVED("iimage3D");
		LEXER_CHECK_TOKEN_WORD_RESERVED("iimageBuffer");
		LEXER_CHECK_TOKEN_WORD_RESERVED("iimageCube");
		LEXER_CHECK_TOKEN_WORD_RESERVED("image1D");
		LEXER_CHECK_TOKEN_WORD_RESERVED("image1DArray");
		LEXER_CHECK_TOKEN_WORD_RESERVED("image1DArrayShadow");
		LEXER_CHECK_TOKEN_WORD_RESERVED("image1DShadow");
		LEXER_CHECK_TOKEN_WORD_RESERVED("image2D");
		LEXER_CHECK_TOKEN_WORD_RESERVED("image2DArray");
		LEXER_CHECK_TOKEN_WORD_RESERVED("image2DArrayShadow");
		LEXER_CHECK_TOKEN_WORD_RESERVED("image2DShadow");
		LEXER_CHECK_TOKEN_WORD_RESERVED("image3D");
		LEXER_CHECK_TOKEN_WORD_RESERVED("imageBuffer");
		LEXER_CHECK_TOKEN_WORD_RESERVED("imageCube");
		LEXER_CHECK_TOKEN_WORD_RESERVED("inline");
		LEXER_CHECK_TOKEN_WORD_RESERVED("input");
		LEXER_CHECK_TOKEN_WORD_RESERVED("interface");
		LEXER_CHECK_TOKEN_WORD_RESERVED("long");
		LEXER_CHECK_TOKEN_WORD_RESERVED("namespace");
		LEXER_CHECK_TOKEN_WORD_RESERVED("noinline");
		LEXER_CHECK_TOKEN_WORD_RESERVED("output");
		LEXER_CHECK_TOKEN_WORD_RESERVED("packed");
		LEXER_CHECK_TOKEN_WORD_RESERVED("partition");
		LEXER_CHECK_TOKEN_WORD_RESERVED("public");
		LEXER_CHECK_TOKEN_WORD_RESERVED("row_major");
		LEXER_CHECK_TOKEN_WORD_RESERVED("sampler3DRect");
		LEXER_CHECK_TOKEN_WORD_RESERVED("short");
		LEXER_CHECK_TOKEN_WORD_RESERVED("sizeof");
		LEXER_CHECK_TOKEN_WORD_RESERVED("static");
		LEXER_CHECK_TOKEN_WORD_RESERVED("superp");
		LEXER_CHECK_TOKEN_WORD_RESERVED("template");
		LEXER_CHECK_TOKEN_WORD_RESERVED("this");
		LEXER_CHECK_TOKEN_WORD_RESERVED("typedef");
		LEXER_CHECK_TOKEN_WORD_RESERVED("uimage1D");
		LEXER_CHECK_TOKEN_WORD_RESERVED("uimage1DArray");
		LEXER_CHECK_TOKEN_WORD_RESERVED("uimage2D");
		LEXER_CHECK_TOKEN_WORD_RESERVED("uimage2DArray");
		LEXER_CHECK_TOKEN_WORD_RESERVED("uimage3D");
		LEXER_CHECK_TOKEN_WORD_RESERVED("uimageBuffer");
		LEXER_CHECK_TOKEN_WORD_RESERVED("uimageCube");
		LEXER_CHECK_TOKEN_WORD_RESERVED("union");
		LEXER_CHECK_TOKEN_WORD_RESERVED("unsigned");
		LEXER_CHECK_TOKEN_WORD_RESERVED("using");
		LEXER_CHECK_TOKEN_WORD_RESERVED("volatile");
		LEXER_CHECK_TOKEN_WORD_VALUE("int",GLSL_LEXER_TOKEN_TYPE_BUILTIN_TYPE,builtin_type=GLSL_BUILTIN_TYPE_INT);
		LEXER_CHECK_TOKEN_WORD_VALUE("void",GLSL_LEXER_TOKEN_TYPE_BUILTIN_TYPE,builtin_type=GLSL_BUILTIN_TYPE_VOID);
		LEXER_CHECK_TOKEN_WORD_VALUE("bool",GLSL_LEXER_TOKEN_TYPE_BUILTIN_TYPE,builtin_type=GLSL_BUILTIN_TYPE_BOOL);
		LEXER_CHECK_TOKEN_WORD_VALUE("float",GLSL_LEXER_TOKEN_TYPE_BUILTIN_TYPE,builtin_type=GLSL_BUILTIN_TYPE_FLOAT);
		LEXER_CHECK_TOKEN_WORD_VALUE("mat2",GLSL_LEXER_TOKEN_TYPE_BUILTIN_TYPE,builtin_type=GLSL_BUILTIN_TYPE_MAT22);
		LEXER_CHECK_TOKEN_WORD_VALUE("mat3",GLSL_LEXER_TOKEN_TYPE_BUILTIN_TYPE,builtin_type=GLSL_BUILTIN_TYPE_MAT33);
		LEXER_CHECK_TOKEN_WORD_VALUE("mat4",GLSL_LEXER_TOKEN_TYPE_BUILTIN_TYPE,builtin_type=GLSL_BUILTIN_TYPE_MAT44);
		LEXER_CHECK_TOKEN_WORD_VALUE("mat2x2",GLSL_LEXER_TOKEN_TYPE_BUILTIN_TYPE,builtin_type=GLSL_BUILTIN_TYPE_MAT22);
		LEXER_CHECK_TOKEN_WORD_VALUE("mat2x3",GLSL_LEXER_TOKEN_TYPE_BUILTIN_TYPE,builtin_type=GLSL_BUILTIN_TYPE_MAT23);
		LEXER_CHECK_TOKEN_WORD_VALUE("mat2x4",GLSL_LEXER_TOKEN_TYPE_BUILTIN_TYPE,builtin_type=GLSL_BUILTIN_TYPE_MAT24);
		LEXER_CHECK_TOKEN_WORD_VALUE("mat3x2",GLSL_LEXER_TOKEN_TYPE_BUILTIN_TYPE,builtin_type=GLSL_BUILTIN_TYPE_MAT32);
		LEXER_CHECK_TOKEN_WORD_VALUE("mat3x3",GLSL_LEXER_TOKEN_TYPE_BUILTIN_TYPE,builtin_type=GLSL_BUILTIN_TYPE_MAT33);
		LEXER_CHECK_TOKEN_WORD_VALUE("mat3x4",GLSL_LEXER_TOKEN_TYPE_BUILTIN_TYPE,builtin_type=GLSL_BUILTIN_TYPE_MAT34);
		LEXER_CHECK_TOKEN_WORD_VALUE("mat4x2",GLSL_LEXER_TOKEN_TYPE_BUILTIN_TYPE,builtin_type=GLSL_BUILTIN_TYPE_MAT42);
		LEXER_CHECK_TOKEN_WORD_VALUE("mat4x3",GLSL_LEXER_TOKEN_TYPE_BUILTIN_TYPE,builtin_type=GLSL_BUILTIN_TYPE_MAT43);
		LEXER_CHECK_TOKEN_WORD_VALUE("mat4x4",GLSL_LEXER_TOKEN_TYPE_BUILTIN_TYPE,builtin_type=GLSL_BUILTIN_TYPE_MAT44);
		LEXER_CHECK_TOKEN_WORD_VALUE("vec2",GLSL_LEXER_TOKEN_TYPE_BUILTIN_TYPE,builtin_type=GLSL_BUILTIN_TYPE_VEC2);
		LEXER_CHECK_TOKEN_WORD_VALUE("vec3",GLSL_LEXER_TOKEN_TYPE_BUILTIN_TYPE,builtin_type=GLSL_BUILTIN_TYPE_VEC3);
		LEXER_CHECK_TOKEN_WORD_VALUE("vec4",GLSL_LEXER_TOKEN_TYPE_BUILTIN_TYPE,builtin_type=GLSL_BUILTIN_TYPE_VEC4);
		LEXER_CHECK_TOKEN_WORD_VALUE("ivec2",GLSL_LEXER_TOKEN_TYPE_BUILTIN_TYPE,builtin_type=GLSL_BUILTIN_TYPE_IVEC2);
		LEXER_CHECK_TOKEN_WORD_VALUE("ivec3",GLSL_LEXER_TOKEN_TYPE_BUILTIN_TYPE,builtin_type=GLSL_BUILTIN_TYPE_IVEC3);
		LEXER_CHECK_TOKEN_WORD_VALUE("ivec4",GLSL_LEXER_TOKEN_TYPE_BUILTIN_TYPE,builtin_type=GLSL_BUILTIN_TYPE_IVEC4);
		LEXER_CHECK_TOKEN_WORD_VALUE("bvec2",GLSL_LEXER_TOKEN_TYPE_BUILTIN_TYPE,builtin_type=GLSL_BUILTIN_TYPE_BVEC2);
		LEXER_CHECK_TOKEN_WORD_VALUE("bvec3",GLSL_LEXER_TOKEN_TYPE_BUILTIN_TYPE,builtin_type=GLSL_BUILTIN_TYPE_BVEC3);
		LEXER_CHECK_TOKEN_WORD_VALUE("bvec4",GLSL_LEXER_TOKEN_TYPE_BUILTIN_TYPE,builtin_type=GLSL_BUILTIN_TYPE_BVEC4);
		LEXER_CHECK_TOKEN_WORD_VALUE("uint",GLSL_LEXER_TOKEN_TYPE_BUILTIN_TYPE,builtin_type=GLSL_BUILTIN_TYPE_UINT);
		LEXER_CHECK_TOKEN_WORD_VALUE("uvec2",GLSL_LEXER_TOKEN_TYPE_BUILTIN_TYPE,builtin_type=GLSL_BUILTIN_TYPE_UVEC2);
		LEXER_CHECK_TOKEN_WORD_VALUE("uvec3",GLSL_LEXER_TOKEN_TYPE_BUILTIN_TYPE,builtin_type=GLSL_BUILTIN_TYPE_UVEC3);
		LEXER_CHECK_TOKEN_WORD_VALUE("uvec4",GLSL_LEXER_TOKEN_TYPE_BUILTIN_TYPE,builtin_type=GLSL_BUILTIN_TYPE_UVEC4);
		LEXER_CHECK_TOKEN_WORD_VALUE("sampler1D",GLSL_LEXER_TOKEN_TYPE_BUILTIN_TYPE,builtin_type=GLSL_BUILTIN_TYPE_SAMPLER_1D);
		LEXER_CHECK_TOKEN_WORD_VALUE("sampler2D",GLSL_LEXER_TOKEN_TYPE_BUILTIN_TYPE,builtin_type=GLSL_BUILTIN_TYPE_SAMPLER_2D);
		LEXER_CHECK_TOKEN_WORD_VALUE("sampler3D",GLSL_LEXER_TOKEN_TYPE_BUILTIN_TYPE,builtin_type=GLSL_BUILTIN_TYPE_SAMPLER_3D);
		LEXER_CHECK_TOKEN_WORD_VALUE("samplerCube",GLSL_LEXER_TOKEN_TYPE_BUILTIN_TYPE,builtin_type=GLSL_BUILTIN_TYPE_SAMPLER_CB);
		LEXER_CHECK_TOKEN_WORD_VALUE("sampler1DShadow",GLSL_LEXER_TOKEN_TYPE_BUILTIN_TYPE,builtin_type=GLSL_BUILTIN_TYPE_SAMPLER_1D_SHADOW);
		LEXER_CHECK_TOKEN_WORD_VALUE("sampler2DShadow",GLSL_LEXER_TOKEN_TYPE_BUILTIN_TYPE,builtin_type=GLSL_BUILTIN_TYPE_SAMPLER_2D_SHADOW);
		LEXER_CHECK_TOKEN_WORD_VALUE("samplerCubeShadow",GLSL_LEXER_TOKEN_TYPE_BUILTIN_TYPE,builtin_type=GLSL_BUILTIN_TYPE_SAMPLER_CB_SHADOW);
		LEXER_CHECK_TOKEN_WORD_VALUE("sampler1DArray",GLSL_LEXER_TOKEN_TYPE_BUILTIN_TYPE,builtin_type=GLSL_BUILTIN_TYPE_SAMPLER_1D_ARRAY);
		LEXER_CHECK_TOKEN_WORD_VALUE("sampler2DArray",GLSL_LEXER_TOKEN_TYPE_BUILTIN_TYPE,builtin_type=GLSL_BUILTIN_TYPE_SAMPLER_2D_ARRAY);
		LEXER_CHECK_TOKEN_WORD_VALUE("sampler1DArrayShadow",GLSL_LEXER_TOKEN_TYPE_BUILTIN_TYPE,builtin_type=GLSL_BUILTIN_TYPE_SAMPLER_1D_ARRAY_SHADOW);
		LEXER_CHECK_TOKEN_WORD_VALUE("sampler2DArrayShadow",GLSL_LEXER_TOKEN_TYPE_BUILTIN_TYPE,builtin_type=GLSL_BUILTIN_TYPE_SAMPLER_2D_ARRAY_SHADOW);
		LEXER_CHECK_TOKEN_WORD_VALUE("isampler1D",GLSL_LEXER_TOKEN_TYPE_BUILTIN_TYPE,builtin_type=GLSL_BUILTIN_TYPE_ISAMPLER_1D);
		LEXER_CHECK_TOKEN_WORD_VALUE("isampler2D",GLSL_LEXER_TOKEN_TYPE_BUILTIN_TYPE,builtin_type=GLSL_BUILTIN_TYPE_ISAMPLER_2D);
		LEXER_CHECK_TOKEN_WORD_VALUE("isampler3D",GLSL_LEXER_TOKEN_TYPE_BUILTIN_TYPE,builtin_type=GLSL_BUILTIN_TYPE_ISAMPLER_3D);
		LEXER_CHECK_TOKEN_WORD_VALUE("isamplerCube",GLSL_LEXER_TOKEN_TYPE_BUILTIN_TYPE,builtin_type=GLSL_BUILTIN_TYPE_ISAMPLER_CB);
		LEXER_CHECK_TOKEN_WORD_VALUE("isampler1DArray",GLSL_LEXER_TOKEN_TYPE_BUILTIN_TYPE,builtin_type=GLSL_BUILTIN_TYPE_ISAMPLER_1D_ARRAY);
		LEXER_CHECK_TOKEN_WORD_VALUE("isampler2DArray",GLSL_LEXER_TOKEN_TYPE_BUILTIN_TYPE,builtin_type=GLSL_BUILTIN_TYPE_ISAMPLER_2D_ARRAY);
		LEXER_CHECK_TOKEN_WORD_VALUE("usampler1D",GLSL_LEXER_TOKEN_TYPE_BUILTIN_TYPE,builtin_type=GLSL_BUILTIN_TYPE_USAMPLER_1D);
		LEXER_CHECK_TOKEN_WORD_VALUE("usampler2D",GLSL_LEXER_TOKEN_TYPE_BUILTIN_TYPE,builtin_type=GLSL_BUILTIN_TYPE_USAMPLER_2D);
		LEXER_CHECK_TOKEN_WORD_VALUE("usampler3D",GLSL_LEXER_TOKEN_TYPE_BUILTIN_TYPE,builtin_type=GLSL_BUILTIN_TYPE_USAMPLER_3D);
		LEXER_CHECK_TOKEN_WORD_VALUE("usamplerCube",GLSL_LEXER_TOKEN_TYPE_BUILTIN_TYPE,builtin_type=GLSL_BUILTIN_TYPE_USAMPLER_CB);
		LEXER_CHECK_TOKEN_WORD_VALUE("usampler1DArray",GLSL_LEXER_TOKEN_TYPE_BUILTIN_TYPE,builtin_type=GLSL_BUILTIN_TYPE_USAMPLER_1D_ARRAY);
		LEXER_CHECK_TOKEN_WORD_VALUE("usampler2DArray",GLSL_LEXER_TOKEN_TYPE_BUILTIN_TYPE,builtin_type=GLSL_BUILTIN_TYPE_USAMPLER_2D_ARRAY);
		LEXER_CHECK_TOKEN_WORD_VALUE("sampler2DRect",GLSL_LEXER_TOKEN_TYPE_BUILTIN_TYPE,builtin_type=GLSL_BUILTIN_TYPE_SAMPLER_2D_RECT);
		LEXER_CHECK_TOKEN_WORD_VALUE("sampler2DRectShadow",GLSL_LEXER_TOKEN_TYPE_BUILTIN_TYPE,builtin_type=GLSL_BUILTIN_TYPE_SAMPLER_2D_RECT_SHADOW);
		LEXER_CHECK_TOKEN_WORD_VALUE("isampler2DRect",GLSL_LEXER_TOKEN_TYPE_BUILTIN_TYPE,builtin_type=GLSL_BUILTIN_TYPE_ISAMPLER_2D_RECT);
		LEXER_CHECK_TOKEN_WORD_VALUE("usampler2DRect",GLSL_LEXER_TOKEN_TYPE_BUILTIN_TYPE,builtin_type=GLSL_BUILTIN_TYPE_USAMPLER_2D_RECT);
		LEXER_CHECK_TOKEN_WORD_VALUE("samplerBuffer",GLSL_LEXER_TOKEN_TYPE_BUILTIN_TYPE,builtin_type=GLSL_BUILTIN_TYPE_SAMPLER_BUFFER);
		LEXER_CHECK_TOKEN_WORD_VALUE("isamplerBuffer",GLSL_LEXER_TOKEN_TYPE_BUILTIN_TYPE,builtin_type=GLSL_BUILTIN_TYPE_ISAMPLER_BUFFER);
		LEXER_CHECK_TOKEN_WORD_VALUE("usamplerBuffer",GLSL_LEXER_TOKEN_TYPE_BUILTIN_TYPE,builtin_type=GLSL_BUILTIN_TYPE_USAMPLER_BUFFER);
		LEXER_CHECK_TOKEN_WORD_VALUE("sampler2DMS",GLSL_LEXER_TOKEN_TYPE_BUILTIN_TYPE,builtin_type=GLSL_BUILTIN_TYPE_SAMPLER_2D_MULTI_SAMPLE);
		LEXER_CHECK_TOKEN_WORD_VALUE("isampler2DMS",GLSL_LEXER_TOKEN_TYPE_BUILTIN_TYPE,builtin_type=GLSL_BUILTIN_TYPE_ISAMPLER_2D_MULTI_SAMPLE);
		LEXER_CHECK_TOKEN_WORD_VALUE("usampler2DMS",GLSL_LEXER_TOKEN_TYPE_BUILTIN_TYPE,builtin_type=GLSL_BUILTIN_TYPE_USAMPLER_2D_MULTI_SAMPLE);
		LEXER_CHECK_TOKEN_WORD_VALUE("sampler2DMSArray",GLSL_LEXER_TOKEN_TYPE_BUILTIN_TYPE,builtin_type=GLSL_BUILTIN_TYPE_SAMPLER_2D_MULTI_SAMPLE_ARRAY);
		LEXER_CHECK_TOKEN_WORD_VALUE("isampler2DMSArray",GLSL_LEXER_TOKEN_TYPE_BUILTIN_TYPE,builtin_type=GLSL_BUILTIN_TYPE_ISAMPLER_2D_MULTI_SAMPLE_ARRAY);
		LEXER_CHECK_TOKEN_WORD_VALUE("usampler2DMSArray",GLSL_LEXER_TOKEN_TYPE_BUILTIN_TYPE,builtin_type=GLSL_BUILTIN_TYPE_USAMPLER_2D_MULTI_SAMPLE_ARRAY);
		LEXER_CHECK_TOKEN_WORD("attribute",GLSL_LEXER_TOKEN_TYPE_ATTRIBUTE);
		LEXER_CHECK_TOKEN_WORD("break",GLSL_LEXER_TOKEN_TYPE_BREAK);
		LEXER_CHECK_TOKEN_WORD("centroid",GLSL_LEXER_TOKEN_TYPE_CENTROID);
		LEXER_CHECK_TOKEN_WORD("const",GLSL_LEXER_TOKEN_TYPE_CONST);
		LEXER_CHECK_TOKEN_WORD("continue",GLSL_LEXER_TOKEN_TYPE_CONTINUE);
		LEXER_CHECK_TOKEN_WORD("discard",GLSL_LEXER_TOKEN_TYPE_DISCARD);
		LEXER_CHECK_TOKEN_WORD("do",GLSL_LEXER_TOKEN_TYPE_DO);
		LEXER_CHECK_TOKEN_WORD("else",GLSL_LEXER_TOKEN_TYPE_ELSE);
		LEXER_CHECK_TOKEN_WORD("flat",GLSL_LEXER_TOKEN_TYPE_FLAT);
		LEXER_CHECK_TOKEN_WORD("for",GLSL_LEXER_TOKEN_TYPE_FOR);
		LEXER_CHECK_TOKEN_WORD("highp",GLSL_LEXER_TOKEN_TYPE_HIGHP);
		LEXER_CHECK_TOKEN_WORD("if",GLSL_LEXER_TOKEN_TYPE_IF);
		LEXER_CHECK_TOKEN_WORD("in",GLSL_LEXER_TOKEN_TYPE_IN);
		LEXER_CHECK_TOKEN_WORD("inout",GLSL_LEXER_TOKEN_TYPE_INOUT);
		LEXER_CHECK_TOKEN_WORD("invariant",GLSL_LEXER_TOKEN_TYPE_INVARIANT);
		LEXER_CHECK_TOKEN_WORD("layout",GLSL_LEXER_TOKEN_TYPE_LAYOUT);
		LEXER_CHECK_TOKEN_WORD("lowp",GLSL_LEXER_TOKEN_TYPE_LOWP);
		LEXER_CHECK_TOKEN_WORD("mediump",GLSL_LEXER_TOKEN_TYPE_MEDIUMP);
		LEXER_CHECK_TOKEN_WORD("noperspective",GLSL_LEXER_TOKEN_TYPE_NOPERSPECTIVE);
		LEXER_CHECK_TOKEN_WORD("out",GLSL_LEXER_TOKEN_TYPE_OUT);
		LEXER_CHECK_TOKEN_WORD("precision",GLSL_LEXER_TOKEN_TYPE_PRECISION);
		LEXER_CHECK_TOKEN_WORD("return",GLSL_LEXER_TOKEN_TYPE_RETURN);
		LEXER_CHECK_TOKEN_WORD("smooth",GLSL_LEXER_TOKEN_TYPE_SMOOTH);
		LEXER_CHECK_TOKEN_WORD("struct",GLSL_LEXER_TOKEN_TYPE_STRUCT);
		LEXER_CHECK_TOKEN_WORD("uniform",GLSL_LEXER_TOKEN_TYPE_UNIFORM);
		LEXER_CHECK_TOKEN_WORD("varying",GLSL_LEXER_TOKEN_TYPE_VARYING);
		LEXER_CHECK_TOKEN_WORD("while",GLSL_LEXER_TOKEN_TYPE_WHILE);
		LEXER_CHECK_TOKEN_WORD_VALUE("false",GLSL_LEXER_TOKEN_TYPE_CONST_BOOL,bool_=0);
		LEXER_CHECK_TOKEN_WORD_VALUE("true",GLSL_LEXER_TOKEN_TYPE_CONST_BOOL,bool_=1);
		LEXER_CHECK_TOKEN("<<=",GLSL_LEXER_TOKEN_TYPE_LSH_ASSIGN);
		LEXER_CHECK_TOKEN(">>=",GLSL_LEXER_TOKEN_TYPE_RSH_ASSIGN);
		LEXER_CHECK_TOKEN("!=",GLSL_LEXER_TOKEN_TYPE_NEQ);
		LEXER_CHECK_TOKEN("%=",GLSL_LEXER_TOKEN_TYPE_MOD_ASSIGN);
		LEXER_CHECK_TOKEN("&&",GLSL_LEXER_TOKEN_TYPE_LAND);
		LEXER_CHECK_TOKEN("*=",GLSL_LEXER_TOKEN_TYPE_MUL_ASSIGN);
		LEXER_CHECK_TOKEN("++",GLSL_LEXER_TOKEN_TYPE_INC);
		LEXER_CHECK_TOKEN("+=",GLSL_LEXER_TOKEN_TYPE_ADD_ASSIGN);
		LEXER_CHECK_TOKEN("--",GLSL_LEXER_TOKEN_TYPE_DEC);
		LEXER_CHECK_TOKEN("-=",GLSL_LEXER_TOKEN_TYPE_SUB_ASSIGN);
		LEXER_CHECK_TOKEN("/=",GLSL_LEXER_TOKEN_TYPE_DIV_ASSIGN);
		LEXER_CHECK_TOKEN("<<",GLSL_LEXER_TOKEN_TYPE_LSH);
		LEXER_CHECK_TOKEN("<=",GLSL_LEXER_TOKEN_TYPE_LEQ);
		LEXER_CHECK_TOKEN("==",GLSL_LEXER_TOKEN_TYPE_EQU);
		LEXER_CHECK_TOKEN(">=",GLSL_LEXER_TOKEN_TYPE_GEQ);
		LEXER_CHECK_TOKEN(">>",GLSL_LEXER_TOKEN_TYPE_RSH);
		LEXER_CHECK_TOKEN("||",GLSL_LEXER_TOKEN_TYPE_LOR);
		LEXER_CHECK_TOKEN("!",GLSL_LEXER_TOKEN_TYPE_NOT);
		LEXER_CHECK_TOKEN("%",GLSL_LEXER_TOKEN_TYPE_MOD);
		LEXER_CHECK_TOKEN("&",GLSL_LEXER_TOKEN_TYPE_AND);
		LEXER_CHECK_TOKEN("(",GLSL_LEXER_TOKEN_TYPE_LEFT_PAREN);
		LEXER_CHECK_TOKEN(")",GLSL_LEXER_TOKEN_TYPE_RIGHT_PAREN);
		LEXER_CHECK_TOKEN("*",GLSL_LEXER_TOKEN_TYPE_MUL);
		LEXER_CHECK_TOKEN("+",GLSL_LEXER_TOKEN_TYPE_ADD);
		LEXER_CHECK_TOKEN(",",GLSL_LEXER_TOKEN_TYPE_COMMA);
		LEXER_CHECK_TOKEN("-",GLSL_LEXER_TOKEN_TYPE_SUB);
		LEXER_CHECK_TOKEN("/",GLSL_LEXER_TOKEN_TYPE_DIV);
		LEXER_CHECK_TOKEN(":",GLSL_LEXER_TOKEN_TYPE_COLON);
		LEXER_CHECK_TOKEN(";",GLSL_LEXER_TOKEN_TYPE_SEMICOLON);
		LEXER_CHECK_TOKEN("<",GLSL_LEXER_TOKEN_TYPE_LESS);
		LEXER_CHECK_TOKEN("=",GLSL_LEXER_TOKEN_TYPE_EQUAL);
		LEXER_CHECK_TOKEN(">",GLSL_LEXER_TOKEN_TYPE_MORE);
		LEXER_CHECK_TOKEN("?",GLSL_LEXER_TOKEN_TYPE_QUESTION_MARK);
		LEXER_CHECK_TOKEN("[",GLSL_LEXER_TOKEN_TYPE_LEFT_BRACKET);
		LEXER_CHECK_TOKEN("]",GLSL_LEXER_TOKEN_TYPE_RIGHT_BRACKET);
		LEXER_CHECK_TOKEN("^",GLSL_LEXER_TOKEN_TYPE_XOR);
		LEXER_CHECK_TOKEN("{",GLSL_LEXER_TOKEN_TYPE_LEFT_BRACE);
		LEXER_CHECK_TOKEN("|",GLSL_LEXER_TOKEN_TYPE_OR);
		LEXER_CHECK_TOKEN("}",GLSL_LEXER_TOKEN_TYPE_RIGHT_BRACE);
		LEXER_CHECK_TOKEN("~",GLSL_LEXER_TOKEN_TYPE_INV);
		if (LEXER_IS_IDENTIFIER_START(src[0])){
			u32 length=0;
			do{
				length++;
			} while (LEXER_IS_IDENTIFIER(src[length]));
			char* string=sys_heap_alloc(NULL,length+1);
			sys_memory_copy(src,string,length);
			string[length]=0;
			_emit_token(out,GLSL_LEXER_TOKEN_TYPE_IDENTIFIER)->string=string;
			src+=length;
			continue;
		}
		if (src[0]=='0'&&(src[1]=='X'||src[1]=='x')){
			src+=2;
			u64 value=0;
			do{
				if (!LEXER_IS_HEX_DIGIT(src[0])){
					error=_glsl_error_create_lexer_digit_expected(src[0],16);
					goto _error;
				}
				value=(value<<4)+src[0]-(LEXER_IS_DIGIT(src[0])?48:55+((src[0]>='a')<<5));
				src++;
			} while (LEXER_IS_IDENTIFIER(src[0])&&src[0]!='u'&&src[0]!='U');
			if (src[0]=='u'||src[0]=='U'){
				src++;
				if (LEXER_IS_IDENTIFIER(src[0])){
					error=_glsl_error_create_lexer_unexpected_character(src[0]);
					goto _error;
				}
			}
			_emit_token(out,GLSL_LEXER_TOKEN_TYPE_CONST_INT)->int_=value;
			continue;
		}
		if (src[0]=='0'&&LEXER_IS_IDENTIFIER(src[1])){
			src++;
			u64 value=0;
			do{
				if (!LEXER_IS_OCT_DIGIT(src[0])){
					error=_glsl_error_create_lexer_digit_expected(src[0],8);
					goto _error;
				}
				value=(value<<3)+src[0]-48;
				src++;
			} while (LEXER_IS_IDENTIFIER(src[0])&&src[0]!='u'&&src[0]!='U');
			if (src[0]=='u'||src[0]=='U'){
				src++;
				if (LEXER_IS_IDENTIFIER(src[0])){
					error=_glsl_error_create_lexer_unexpected_character(src[0]);
					goto _error;
				}
			}
			_emit_token(out,GLSL_LEXER_TOKEN_TYPE_CONST_INT)->int_=value;
			continue;
		}
		if (LEXER_IS_DIGIT(src[0])||(src[0]=='.'&&LEXER_IS_DIGIT(src[1]))){
			u64 integer_part=0;
			while (LEXER_IS_IDENTIFIER(src[0])&&src[0]!='e'&&src[0]!='E'&&src[0]!='u'&&src[0]!='U'){
				if (!LEXER_IS_DIGIT(src[0])){
					error=_glsl_error_create_lexer_digit_expected(src[0],10);
					goto _error;
				}
				integer_part=integer_part*10+src[0]-48;
				src++;
			}
			if (src[0]!='.'&&src[0]!='e'&&src[0]!='E'){
				if (src[0]=='u'||src[0]=='U'){
					src++;
					if (LEXER_IS_IDENTIFIER(src[0])){
						error=_glsl_error_create_lexer_unexpected_character(src[0]);
						goto _error;
					}
				}
				_emit_token(out,GLSL_LEXER_TOKEN_TYPE_CONST_INT)->int_=integer_part;
				continue;
			}
			double value=integer_part;
			if (src[0]=='.'){
				double power=0.1;
				src++;
				while (LEXER_IS_IDENTIFIER(src[0])&&src[0]!='e'&&src[0]!='E'){
					if (!LEXER_IS_DIGIT(src[0])){
						error=_glsl_error_create_lexer_digit_expected(src[0],10);
						goto _error;
					}
					value+=(src[0]-48)*power;
					power*=0.1;
					src++;
				}
			}
			if (src[0]=='e'||src[0]=='E'){
				src++;
				_Bool is_negative=0;
				if (src[0]=='+'){
					src++;
				}
				else if (src[0]=='-'){
					is_negative=1;
					src++;
				}
				s64 exponent=0;
				do{
					if (!LEXER_IS_DIGIT(src[0])){
						error=_glsl_error_create_lexer_digit_expected(src[0],10);
						goto _error;
					}
					exponent=exponent*10+src[0]-48;
					src++;
				} while (LEXER_IS_IDENTIFIER(src[0]));
				sys_io_print("libmath.so not found\n");
				(void)is_negative;// value*=pow(10,exponent*(is_negative?-1:1));
			}
			if (LEXER_IS_IDENTIFIER(src[0])){
				error=_glsl_error_create_lexer_unexpected_character(src[0]);
				goto _error;
			}
			_emit_token(out,GLSL_LEXER_TOKEN_TYPE_CONST_FLOAT)->float_=value;
			continue;
		}
		LEXER_CHECK_TOKEN(".",GLSL_LEXER_TOKEN_TYPE_PERIOD); // down here because of '.12345' float syntax
		error=_glsl_error_create_lexer_unexpected_character(src[0]);
		goto _error;
	}
	if (out->length!=out->_capacity){
		out->data=sys_heap_realloc(NULL,out->data,out->length*sizeof(glsl_lexer_token_t));
		out->_capacity=out->length;
	}
	return GLSL_NO_ERROR;
_error:
	glsl_lexer_delete_token_list(out);
	return error;
}



SYS_PUBLIC void glsl_lexer_delete_token_list(glsl_lexer_token_list_t* token_list){
	for (u32 i=0;i<token_list->length;i++){
		if ((token_list->data+i)->type==GLSL_LEXER_TOKEN_TYPE_IDENTIFIER){
			sys_heap_dealloc(NULL,(token_list->data+i)->string);
		}
	}
	sys_heap_dealloc(NULL,token_list->data);
	token_list->data=NULL;
	token_list->length=0;
	token_list->_capacity=0;
}
