#ifndef _GLSL__INTERNAL_LEXER_H_
#define _GLSL__INTERNAL_LEXER_H_ 1



#define LEXER_IS_WHITESPACE(c) ((c)==' '||(c)=='\t'||(c)=='\n'||(c)=='\r'||c=='\v')
#define LEXER_IS_OCT_DIGIT(c) ((c)>='0'&&(c)<='7')
#define LEXER_IS_DIGIT(c) ((c)>='0'&&(c)<='9')
#define LEXER_IS_HEX_DIGIT(c) (LEXER_IS_DIGIT((c))||((c)>='A'&&(c)<='F')||((c)>='a'&&(c)<='f'))
#define LEXER_IS_IDENTIFIER_START(c) (((c)>='A'&&(c)<='Z')||((c)>='a'&&(c)<='z')||(c)=='_')
#define LEXER_IS_IDENTIFIER(c) (LEXER_IS_IDENTIFIER_START((c))||LEXER_IS_DIGIT((c)))



#endif
