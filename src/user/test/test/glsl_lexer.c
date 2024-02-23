#include <glsl/error.h>
#include <glsl/lexer.h>
#include <sys/string/string.h>
#include <sys/types.h>
#include <test/test.h>



static _Bool _check_and_cleanup_error(glsl_error_t error,const char* expected){
	TEST_ASSERT(error);
	_Bool out=0;
	if (error){
		out=!sys_string_compare(expected,error);
		glsl_error_delete(error);
	}
	return out;
}



void test_glsl_lexer(void){
	TEST_MODULE("glsl_lexer");
	TEST_FUNC("glsl_lexer_extract_tokens");
	TEST_GROUP("empty input");
	glsl_lexer_token_list_t token_list;
	TEST_ASSERT(!glsl_lexer_extract_tokens("",&token_list));
	TEST_ASSERT(!token_list.length);
	glsl_lexer_delete_token_list(&token_list);
	TEST_GROUP("whitespace");
	TEST_ASSERT(!glsl_lexer_extract_tokens(" \t\n\r",&token_list));
	TEST_ASSERT(!token_list.length);
	glsl_lexer_delete_token_list(&token_list);
	TEST_GROUP("reserved keywords");
	TEST_ASSERT(_check_and_cleanup_error(glsl_lexer_extract_tokens("active",&token_list),"Reserved keyword: active"));
	TEST_ASSERT(_check_and_cleanup_error(glsl_lexer_extract_tokens("asm",&token_list),"Reserved keyword: asm"));
	TEST_ASSERT(_check_and_cleanup_error(glsl_lexer_extract_tokens("cast",&token_list),"Reserved keyword: cast"));
	TEST_ASSERT(_check_and_cleanup_error(glsl_lexer_extract_tokens("class",&token_list),"Reserved keyword: class"));
	TEST_ASSERT(_check_and_cleanup_error(glsl_lexer_extract_tokens("common",&token_list),"Reserved keyword: common"));
	TEST_ASSERT(_check_and_cleanup_error(glsl_lexer_extract_tokens("double",&token_list),"Reserved keyword: double"));
	TEST_ASSERT(_check_and_cleanup_error(glsl_lexer_extract_tokens("dvec2",&token_list),"Reserved keyword: dvec2"));
	TEST_ASSERT(_check_and_cleanup_error(glsl_lexer_extract_tokens("dvec3",&token_list),"Reserved keyword: dvec3"));
	TEST_ASSERT(_check_and_cleanup_error(glsl_lexer_extract_tokens("dvec4",&token_list),"Reserved keyword: dvec4"));
	TEST_ASSERT(_check_and_cleanup_error(glsl_lexer_extract_tokens("enum",&token_list),"Reserved keyword: enum"));
	TEST_ASSERT(_check_and_cleanup_error(glsl_lexer_extract_tokens("extern",&token_list),"Reserved keyword: extern"));
	TEST_ASSERT(_check_and_cleanup_error(glsl_lexer_extract_tokens("external",&token_list),"Reserved keyword: external"));
	TEST_ASSERT(_check_and_cleanup_error(glsl_lexer_extract_tokens("filter",&token_list),"Reserved keyword: filter"));
	TEST_ASSERT(_check_and_cleanup_error(glsl_lexer_extract_tokens("fixed",&token_list),"Reserved keyword: fixed"));
	TEST_ASSERT(_check_and_cleanup_error(glsl_lexer_extract_tokens("fvec2",&token_list),"Reserved keyword: fvec2"));
	TEST_ASSERT(_check_and_cleanup_error(glsl_lexer_extract_tokens("fvec3",&token_list),"Reserved keyword: fvec3"));
	TEST_ASSERT(_check_and_cleanup_error(glsl_lexer_extract_tokens("fvec4",&token_list),"Reserved keyword: fvec4"));
	TEST_ASSERT(_check_and_cleanup_error(glsl_lexer_extract_tokens("goto",&token_list),"Reserved keyword: goto"));
	TEST_ASSERT(_check_and_cleanup_error(glsl_lexer_extract_tokens("half",&token_list),"Reserved keyword: half"));
	TEST_ASSERT(_check_and_cleanup_error(glsl_lexer_extract_tokens("hvec2",&token_list),"Reserved keyword: hvec2"));
	TEST_ASSERT(_check_and_cleanup_error(glsl_lexer_extract_tokens("hvec3",&token_list),"Reserved keyword: hvec3"));
	TEST_ASSERT(_check_and_cleanup_error(glsl_lexer_extract_tokens("hvec4",&token_list),"Reserved keyword: hvec4"));
	TEST_ASSERT(_check_and_cleanup_error(glsl_lexer_extract_tokens("iimage1D",&token_list),"Reserved keyword: iimage1D"));
	TEST_ASSERT(_check_and_cleanup_error(glsl_lexer_extract_tokens("iimage1DArray",&token_list),"Reserved keyword: iimage1DArray"));
	TEST_ASSERT(_check_and_cleanup_error(glsl_lexer_extract_tokens("iimage2D",&token_list),"Reserved keyword: iimage2D"));
	TEST_ASSERT(_check_and_cleanup_error(glsl_lexer_extract_tokens("iimage2DArray",&token_list),"Reserved keyword: iimage2DArray"));
	TEST_ASSERT(_check_and_cleanup_error(glsl_lexer_extract_tokens("iimage3D",&token_list),"Reserved keyword: iimage3D"));
	TEST_ASSERT(_check_and_cleanup_error(glsl_lexer_extract_tokens("iimageBuffer",&token_list),"Reserved keyword: iimageBuffer"));
	TEST_ASSERT(_check_and_cleanup_error(glsl_lexer_extract_tokens("iimageCube",&token_list),"Reserved keyword: iimageCube"));
	TEST_ASSERT(_check_and_cleanup_error(glsl_lexer_extract_tokens("image1D",&token_list),"Reserved keyword: image1D"));
	TEST_ASSERT(_check_and_cleanup_error(glsl_lexer_extract_tokens("image1DArray",&token_list),"Reserved keyword: image1DArray"));
	TEST_ASSERT(_check_and_cleanup_error(glsl_lexer_extract_tokens("image1DArrayShadow",&token_list),"Reserved keyword: image1DArrayShadow"));
	TEST_ASSERT(_check_and_cleanup_error(glsl_lexer_extract_tokens("image1DShadow",&token_list),"Reserved keyword: image1DShadow"));
	TEST_ASSERT(_check_and_cleanup_error(glsl_lexer_extract_tokens("image2D",&token_list),"Reserved keyword: image2D"));
	TEST_ASSERT(_check_and_cleanup_error(glsl_lexer_extract_tokens("image2DArray",&token_list),"Reserved keyword: image2DArray"));
	TEST_ASSERT(_check_and_cleanup_error(glsl_lexer_extract_tokens("image2DArrayShadow",&token_list),"Reserved keyword: image2DArrayShadow"));
	TEST_ASSERT(_check_and_cleanup_error(glsl_lexer_extract_tokens("image2DShadow",&token_list),"Reserved keyword: image2DShadow"));
	TEST_ASSERT(_check_and_cleanup_error(glsl_lexer_extract_tokens("image3D",&token_list),"Reserved keyword: image3D"));
	TEST_ASSERT(_check_and_cleanup_error(glsl_lexer_extract_tokens("imageBuffer",&token_list),"Reserved keyword: imageBuffer"));
	TEST_ASSERT(_check_and_cleanup_error(glsl_lexer_extract_tokens("imageCube",&token_list),"Reserved keyword: imageCube"));
	TEST_ASSERT(_check_and_cleanup_error(glsl_lexer_extract_tokens("inline",&token_list),"Reserved keyword: inline"));
	TEST_ASSERT(_check_and_cleanup_error(glsl_lexer_extract_tokens("input",&token_list),"Reserved keyword: input"));
	TEST_ASSERT(_check_and_cleanup_error(glsl_lexer_extract_tokens("interface",&token_list),"Reserved keyword: interface"));
	TEST_ASSERT(_check_and_cleanup_error(glsl_lexer_extract_tokens("long",&token_list),"Reserved keyword: long"));
	TEST_ASSERT(_check_and_cleanup_error(glsl_lexer_extract_tokens("namespace",&token_list),"Reserved keyword: namespace"));
	TEST_ASSERT(_check_and_cleanup_error(glsl_lexer_extract_tokens("noinline",&token_list),"Reserved keyword: noinline"));
	TEST_ASSERT(_check_and_cleanup_error(glsl_lexer_extract_tokens("output",&token_list),"Reserved keyword: output"));
	TEST_ASSERT(_check_and_cleanup_error(glsl_lexer_extract_tokens("packed",&token_list),"Reserved keyword: packed"));
	TEST_ASSERT(_check_and_cleanup_error(glsl_lexer_extract_tokens("partition",&token_list),"Reserved keyword: partition"));
	TEST_ASSERT(_check_and_cleanup_error(glsl_lexer_extract_tokens("public",&token_list),"Reserved keyword: public"));
	TEST_ASSERT(_check_and_cleanup_error(glsl_lexer_extract_tokens("row_major",&token_list),"Reserved keyword: row_major"));
	TEST_ASSERT(_check_and_cleanup_error(glsl_lexer_extract_tokens("sampler3DRect",&token_list),"Reserved keyword: sampler3DRect"));
	TEST_ASSERT(_check_and_cleanup_error(glsl_lexer_extract_tokens("short",&token_list),"Reserved keyword: short"));
	TEST_ASSERT(_check_and_cleanup_error(glsl_lexer_extract_tokens("sizeof",&token_list),"Reserved keyword: sizeof"));
	TEST_ASSERT(_check_and_cleanup_error(glsl_lexer_extract_tokens("static",&token_list),"Reserved keyword: static"));
	TEST_ASSERT(_check_and_cleanup_error(glsl_lexer_extract_tokens("superp",&token_list),"Reserved keyword: superp"));
	TEST_ASSERT(_check_and_cleanup_error(glsl_lexer_extract_tokens("template",&token_list),"Reserved keyword: template"));
	TEST_ASSERT(_check_and_cleanup_error(glsl_lexer_extract_tokens("this",&token_list),"Reserved keyword: this"));
	TEST_ASSERT(_check_and_cleanup_error(glsl_lexer_extract_tokens("typedef",&token_list),"Reserved keyword: typedef"));
	TEST_ASSERT(_check_and_cleanup_error(glsl_lexer_extract_tokens("uimage1D",&token_list),"Reserved keyword: uimage1D"));
	TEST_ASSERT(_check_and_cleanup_error(glsl_lexer_extract_tokens("uimage1DArray",&token_list),"Reserved keyword: uimage1DArray"));
	TEST_ASSERT(_check_and_cleanup_error(glsl_lexer_extract_tokens("uimage2D",&token_list),"Reserved keyword: uimage2D"));
	TEST_ASSERT(_check_and_cleanup_error(glsl_lexer_extract_tokens("uimage2DArray",&token_list),"Reserved keyword: uimage2DArray"));
	TEST_ASSERT(_check_and_cleanup_error(glsl_lexer_extract_tokens("uimage3D",&token_list),"Reserved keyword: uimage3D"));
	TEST_ASSERT(_check_and_cleanup_error(glsl_lexer_extract_tokens("uimageBuffer",&token_list),"Reserved keyword: uimageBuffer"));
	TEST_ASSERT(_check_and_cleanup_error(glsl_lexer_extract_tokens("uimageCube",&token_list),"Reserved keyword: uimageCube"));
	TEST_ASSERT(_check_and_cleanup_error(glsl_lexer_extract_tokens("union",&token_list),"Reserved keyword: union"));
	TEST_ASSERT(_check_and_cleanup_error(glsl_lexer_extract_tokens("unsigned",&token_list),"Reserved keyword: unsigned"));
	TEST_ASSERT(_check_and_cleanup_error(glsl_lexer_extract_tokens("using",&token_list),"Reserved keyword: using"));
	TEST_ASSERT(_check_and_cleanup_error(glsl_lexer_extract_tokens("volatile",&token_list),"Reserved keyword: volatile"));
	TEST_ASSERT(!token_list.length);
	glsl_lexer_delete_token_list(&token_list);
}
