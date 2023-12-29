#include <GL/gl.h>
#include <sys/io.h>



SYS_PUBLIC const GLubyte* APIENTRY glGetString(GLenum name){
	printf("Unimplemented: glGetString\n");
	return NULL;
}



SYS_PUBLIC const GLubyte* APIENTRY glGetStringi(GLenum name,GLuint index){
	printf("Unimplemented: glGetStringi\n");
	return NULL;
}



SYS_PUBLIC GLboolean APIENTRY glIsBuffer(GLuint buffer){
	printf("Unimplemented: glIsBuffer\n");
	return 0;
}



SYS_PUBLIC GLboolean APIENTRY glIsEnabled(GLenum cap){
	printf("Unimplemented: glIsEnabled\n");
	return 0;
}



SYS_PUBLIC GLboolean APIENTRY glIsEnabledi(GLenum target,GLuint index){
	printf("Unimplemented: glIsEnabledi\n");
	return 0;
}



SYS_PUBLIC GLboolean APIENTRY glIsFramebuffer(GLuint framebuffer){
	printf("Unimplemented: glIsFramebuffer\n");
	return 0;
}



SYS_PUBLIC GLboolean APIENTRY glIsProgram(GLuint program){
	printf("Unimplemented: glIsProgram\n");
	return 0;
}



SYS_PUBLIC GLboolean APIENTRY glIsQuery(GLuint id){
	printf("Unimplemented: glIsQuery\n");
	return 0;
}



SYS_PUBLIC GLboolean APIENTRY glIsRenderbuffer(GLuint renderbuffer){
	printf("Unimplemented: glIsRenderbuffer\n");
	return 0;
}



SYS_PUBLIC GLboolean APIENTRY glIsSampler(GLuint sampler){
	printf("Unimplemented: glIsSampler\n");
	return 0;
}



SYS_PUBLIC GLboolean APIENTRY glIsShader(GLuint shader){
	printf("Unimplemented: glIsShader\n");
	return 0;
}



SYS_PUBLIC GLboolean APIENTRY glIsSync(GLsync sync){
	printf("Unimplemented: glIsSync\n");
	return 0;
}



SYS_PUBLIC GLboolean APIENTRY glIsTexture(GLuint texture){
	printf("Unimplemented: glIsTexture\n");
	return 0;
}



SYS_PUBLIC GLboolean APIENTRY glIsVertexArray(GLuint array){
	printf("Unimplemented: glIsVertexArray\n");
	return 0;
}



SYS_PUBLIC GLboolean APIENTRY glUnmapBuffer(GLenum target){
	printf("Unimplemented: glUnmapBuffer\n");
	return 0;
}



SYS_PUBLIC GLenum APIENTRY glCheckFramebufferStatus(GLenum target){
	printf("Unimplemented: glCheckFramebufferStatus\n");
	return 0;
}



SYS_PUBLIC GLenum APIENTRY glClientWaitSync(GLsync sync,GLbitfield flags,GLuint64 timeout){
	printf("Unimplemented: glClientWaitSync\n");
	return 0;
}



SYS_PUBLIC GLenum APIENTRY glGetError(void){
	printf("Unimplemented: glGetError\n");
	return 0;
}



SYS_PUBLIC GLint APIENTRY glGetAttribLocation(GLuint program,const GLchar* name){
	printf("Unimplemented: glGetAttribLocation\n");
	return 0;
}



SYS_PUBLIC GLint APIENTRY glGetFragDataIndex(GLuint program,const GLchar* name){
	printf("Unimplemented: glGetFragDataIndex\n");
	return 0;
}



SYS_PUBLIC GLint APIENTRY glGetFragDataLocation(GLuint program,const GLchar* name){
	printf("Unimplemented: glGetFragDataLocation\n");
	return 0;
}



SYS_PUBLIC GLint APIENTRY glGetUniformLocation(GLuint program,const GLchar* name){
	printf("Unimplemented: glGetUniformLocation\n");
	return 0;
}



SYS_PUBLIC GLsync APIENTRY glFenceSync(GLenum condition,GLbitfield flags){
	printf("Unimplemented: glFenceSync\n");
	return 0;
}



SYS_PUBLIC GLuint APIENTRY glCreateProgram(void){
	printf("Unimplemented: glCreateProgram\n");
	return 0;
}



SYS_PUBLIC GLuint APIENTRY glCreateShader(GLenum type){
	printf("Unimplemented: glCreateShader\n");
	return 0;
}



SYS_PUBLIC GLuint APIENTRY glGetUniformBlockIndex(GLuint program,const GLchar* uniformBlockName){
	printf("Unimplemented: glGetUniformBlockIndex\n");
	return 0;
}



SYS_PUBLIC void* APIENTRY glMapBuffer(GLenum target,GLenum access){
	printf("Unimplemented: glMapBuffer\n");
	return NULL;
}



SYS_PUBLIC void* APIENTRY glMapBufferRange(GLenum target,GLintptr offset,GLsizeiptr length,GLbitfield access){
	printf("Unimplemented: glMapBufferRange\n");
	return NULL;
}



SYS_PUBLIC void APIENTRY glActiveTexture(GLenum texture){
	printf("Unimplemented: glActiveTexture\n");
}



SYS_PUBLIC void APIENTRY glAttachShader(GLuint program,GLuint shader){
	printf("Unimplemented: glAttachShader\n");
}



SYS_PUBLIC void APIENTRY glBeginConditionalRender(GLuint id,GLenum mode){
	printf("Unimplemented: glBeginConditionalRender\n");
}



SYS_PUBLIC void APIENTRY glBeginQuery(GLenum target,GLuint id){
	printf("Unimplemented: glBeginQuery\n");
}



SYS_PUBLIC void APIENTRY glBeginTransformFeedback(GLenum primitiveMode){
	printf("Unimplemented: glBeginTransformFeedback\n");
}



SYS_PUBLIC void APIENTRY glBindAttribLocation(GLuint program,GLuint index,const GLchar* name){
	printf("Unimplemented: glBindAttribLocation\n");
}



SYS_PUBLIC void APIENTRY glBindBuffer(GLenum target,GLuint buffer){
	printf("Unimplemented: glBindBuffer\n");
}



SYS_PUBLIC void APIENTRY glBindBufferBase(GLenum target,GLuint index,GLuint buffer){
	printf("Unimplemented: glBindBufferBase\n");
}



SYS_PUBLIC void APIENTRY glBindBufferRange(GLenum target,GLuint index,GLuint buffer,GLintptr offset,GLsizeiptr size){
	printf("Unimplemented: glBindBufferRange\n");
}



SYS_PUBLIC void APIENTRY glBindFragDataLocation(GLuint program,GLuint color,const GLchar* name){
	printf("Unimplemented: glBindFragDataLocation\n");
}



SYS_PUBLIC void APIENTRY glBindFragDataLocationIndexed(GLuint program,GLuint colorNumber,GLuint index,const GLchar* name){
	printf("Unimplemented: glBindFragDataLocationIndexed\n");
}



SYS_PUBLIC void APIENTRY glBindFramebuffer(GLenum target,GLuint framebuffer){
	printf("Unimplemented: glBindFramebuffer\n");
}



SYS_PUBLIC void APIENTRY glBindRenderbuffer(GLenum target,GLuint renderbuffer){
	printf("Unimplemented: glBindRenderbuffer\n");
}



SYS_PUBLIC void APIENTRY glBindSampler(GLuint unit,GLuint sampler){
	printf("Unimplemented: glBindSampler\n");
}



SYS_PUBLIC void APIENTRY glBindTexture(GLenum target,GLuint texture){
	printf("Unimplemented: glBindTexture\n");
}



SYS_PUBLIC void APIENTRY glBindVertexArray(GLuint array){
	printf("Unimplemented: glBindVertexArray\n");
}



SYS_PUBLIC void APIENTRY glBlendColor(GLfloat red,GLfloat green,GLfloat blue,GLfloat alpha){
	printf("Unimplemented: glBlendColor\n");
}



SYS_PUBLIC void APIENTRY glBlendEquation(GLenum mode){
	printf("Unimplemented: glBlendEquation\n");
}



SYS_PUBLIC void APIENTRY glBlendEquationSeparate(GLenum modeRGB,GLenum modeAlpha){
	printf("Unimplemented: glBlendEquationSeparate\n");
}



SYS_PUBLIC void APIENTRY glBlendFunc(GLenum sfactor,GLenum dfactor){
	printf("Unimplemented: glBlendFunc\n");
}



SYS_PUBLIC void APIENTRY glBlendFuncSeparate(GLenum sfactorRGB,GLenum dfactorRGB,GLenum sfactorAlpha,GLenum dfactorAlpha){
	printf("Unimplemented: glBlendFuncSeparate\n");
}



SYS_PUBLIC void APIENTRY glBlitFramebuffer(GLint srcX0,GLint srcY0,GLint srcX1,GLint srcY1,GLint dstX0,GLint dstY0,GLint dstX1,GLint dstY1,GLbitfield mask,GLenum filter){
	printf("Unimplemented: glBlitFramebuffer\n");
}



SYS_PUBLIC void APIENTRY glBufferData(GLenum target,GLsizeiptr size,const void* data,GLenum usage){
	printf("Unimplemented: glBufferData\n");
}



SYS_PUBLIC void APIENTRY glBufferSubData(GLenum target,GLintptr offset,GLsizeiptr size,const void* data){
	printf("Unimplemented: glBufferSubData\n");
}



SYS_PUBLIC void APIENTRY glClampColor(GLenum target,GLenum clamp){
	printf("Unimplemented: glClampColor\n");
}



SYS_PUBLIC void APIENTRY glClear(GLbitfield mask){
	printf("Unimplemented: glClear\n");
}



SYS_PUBLIC void APIENTRY glClearBufferfi(GLenum buffer,GLint drawbuffer,GLfloat depth,GLint stencil){
	printf("Unimplemented: glClearBufferfi\n");
}



SYS_PUBLIC void APIENTRY glClearBufferfv(GLenum buffer,GLint drawbuffer,const GLfloat* value){
	printf("Unimplemented: glClearBufferfv\n");
}



SYS_PUBLIC void APIENTRY glClearBufferiv(GLenum buffer,GLint drawbuffer,const GLint* value){
	printf("Unimplemented: glClearBufferiv\n");
}



SYS_PUBLIC void APIENTRY glClearBufferuiv(GLenum buffer,GLint drawbuffer,const GLuint* value){
	printf("Unimplemented: glClearBufferuiv\n");
}



SYS_PUBLIC void APIENTRY glClearColor(GLfloat red,GLfloat green,GLfloat blue,GLfloat alpha){
	printf("Unimplemented: glClearColor\n");
}



SYS_PUBLIC void APIENTRY glClearDepth(GLdouble depth){
	printf("Unimplemented: glClearDepth\n");
}



SYS_PUBLIC void APIENTRY glClearStencil(GLint s){
	printf("Unimplemented: glClearStencil\n");
}



SYS_PUBLIC void APIENTRY glColorMask(GLboolean red,GLboolean green,GLboolean blue,GLboolean alpha){
	printf("Unimplemented: glColorMask\n");
}



SYS_PUBLIC void APIENTRY glColorMaski(GLuint index,GLboolean r,GLboolean g,GLboolean b,GLboolean a){
	printf("Unimplemented: glColorMaski\n");
}



SYS_PUBLIC void APIENTRY glCompileShader(GLuint shader){
	printf("Unimplemented: glCompileShader\n");
}



SYS_PUBLIC void APIENTRY glCompressedTexImage1D(GLenum target,GLint level,GLenum internalformat,GLsizei width,GLint border,GLsizei imageSize,const void* data){
	printf("Unimplemented: glCompressedTexImage1D\n");
}



SYS_PUBLIC void APIENTRY glCompressedTexImage2D(GLenum target,GLint level,GLenum internalformat,GLsizei width,GLsizei height,GLint border,GLsizei imageSize,const void* data){
	printf("Unimplemented: glCompressedTexImage2D\n");
}



SYS_PUBLIC void APIENTRY glCompressedTexImage3D(GLenum target,GLint level,GLenum internalformat,GLsizei width,GLsizei height,GLsizei depth,GLint border,GLsizei imageSize,const void* data){
	printf("Unimplemented: glCompressedTexImage3D\n");
}



SYS_PUBLIC void APIENTRY glCompressedTexSubImage1D(GLenum target,GLint level,GLint xoffset,GLsizei width,GLenum format,GLsizei imageSize,const void* data){
	printf("Unimplemented: glCompressedTexSubImage1D\n");
}



SYS_PUBLIC void APIENTRY glCompressedTexSubImage2D(GLenum target,GLint level,GLint xoffset,GLint yoffset,GLsizei width,GLsizei height,GLenum format,GLsizei imageSize,const void* data){
	printf("Unimplemented: glCompressedTexSubImage2D\n");
}



SYS_PUBLIC void APIENTRY glCompressedTexSubImage3D(GLenum target,GLint level,GLint xoffset,GLint yoffset,GLint zoffset,GLsizei width,GLsizei height,GLsizei depth,GLenum format,GLsizei imageSize,const void* data){
	printf("Unimplemented: glCompressedTexSubImage3D\n");
}



SYS_PUBLIC void APIENTRY glCopyBufferSubData(GLenum readTarget,GLenum writeTarget,GLintptr readOffset,GLintptr writeOffset,GLsizeiptr size){
	printf("Unimplemented: glCopyBufferSubData\n");
}



SYS_PUBLIC void APIENTRY glCopyTexImage1D(GLenum target,GLint level,GLenum internalformat,GLint x,GLint y,GLsizei width,GLint border){
	printf("Unimplemented: glCopyTexImage1D\n");
}



SYS_PUBLIC void APIENTRY glCopyTexImage2D(GLenum target,GLint level,GLenum internalformat,GLint x,GLint y,GLsizei width,GLsizei height,GLint border){
	printf("Unimplemented: glCopyTexImage2D\n");
}



SYS_PUBLIC void APIENTRY glCopyTexSubImage1D(GLenum target,GLint level,GLint xoffset,GLint x,GLint y,GLsizei width){
	printf("Unimplemented: glCopyTexSubImage1D\n");
}



SYS_PUBLIC void APIENTRY glCopyTexSubImage2D(GLenum target,GLint level,GLint xoffset,GLint yoffset,GLint x,GLint y,GLsizei width,GLsizei height){
	printf("Unimplemented: glCopyTexSubImage2D\n");
}



SYS_PUBLIC void APIENTRY glCopyTexSubImage3D(GLenum target,GLint level,GLint xoffset,GLint yoffset,GLint zoffset,GLint x,GLint y,GLsizei width,GLsizei height){
	printf("Unimplemented: glCopyTexSubImage3D\n");
}



SYS_PUBLIC void APIENTRY glCullFace(GLenum mode){
	printf("Unimplemented: glCullFace\n");
}



SYS_PUBLIC void APIENTRY glDeleteBuffers(GLsizei n,const GLuint* buffers){
	printf("Unimplemented: glDeleteBuffers\n");
}



SYS_PUBLIC void APIENTRY glDeleteFramebuffers(GLsizei n,const GLuint* framebuffers){
	printf("Unimplemented: glDeleteFramebuffers\n");
}



SYS_PUBLIC void APIENTRY glDeleteProgram(GLuint program){
	printf("Unimplemented: glDeleteProgram\n");
}



SYS_PUBLIC void APIENTRY glDeleteQueries(GLsizei n,const GLuint* ids){
	printf("Unimplemented: glDeleteQueries\n");
}



SYS_PUBLIC void APIENTRY glDeleteRenderbuffers(GLsizei n,const GLuint* renderbuffers){
	printf("Unimplemented: glDeleteRenderbuffers\n");
}



SYS_PUBLIC void APIENTRY glDeleteSamplers(GLsizei count,const GLuint* samplers){
	printf("Unimplemented: glDeleteSamplers\n");
}



SYS_PUBLIC void APIENTRY glDeleteShader(GLuint shader){
	printf("Unimplemented: glDeleteShader\n");
}



SYS_PUBLIC void APIENTRY glDeleteSync(GLsync sync){
	printf("Unimplemented: glDeleteSync\n");
}



SYS_PUBLIC void APIENTRY glDeleteTextures(GLsizei n,const GLuint* textures){
	printf("Unimplemented: glDeleteTextures\n");
}



SYS_PUBLIC void APIENTRY glDeleteVertexArrays(GLsizei n,const GLuint* arrays){
	printf("Unimplemented: glDeleteVertexArrays\n");
}



SYS_PUBLIC void APIENTRY glDepthFunc(GLenum func){
	printf("Unimplemented: glDepthFunc\n");
}



SYS_PUBLIC void APIENTRY glDepthMask(GLboolean flag){
	printf("Unimplemented: glDepthMask\n");
}



SYS_PUBLIC void APIENTRY glDepthRange(GLdouble n,GLdouble f){
	printf("Unimplemented: glDepthRange\n");
}



SYS_PUBLIC void APIENTRY glDetachShader(GLuint program,GLuint shader){
	printf("Unimplemented: glDetachShader\n");
}



SYS_PUBLIC void APIENTRY glDisable(GLenum cap){
	printf("Unimplemented: glDisable\n");
}



SYS_PUBLIC void APIENTRY glDisablei(GLenum target,GLuint index){
	printf("Unimplemented: glDisablei\n");
}



SYS_PUBLIC void APIENTRY glDisableVertexAttribArray(GLuint index){
	printf("Unimplemented: glDisableVertexAttribArray\n");
}



SYS_PUBLIC void APIENTRY glDrawArrays(GLenum mode,GLint first,GLsizei count){
	printf("Unimplemented: glDrawArrays\n");
}



SYS_PUBLIC void APIENTRY glDrawArraysInstanced(GLenum mode,GLint first,GLsizei count,GLsizei instancecount){
	printf("Unimplemented: glDrawArraysInstanced\n");
}



SYS_PUBLIC void APIENTRY glDrawBuffer(GLenum buf){
	printf("Unimplemented: glDrawBuffer\n");
}



SYS_PUBLIC void APIENTRY glDrawBuffers(GLsizei n,const GLenum* bufs){
	printf("Unimplemented: glDrawBuffers\n");
}



SYS_PUBLIC void APIENTRY glDrawElements(GLenum mode,GLsizei count,GLenum type,const void* indices){
	printf("Unimplemented: glDrawElements\n");
}



SYS_PUBLIC void APIENTRY glDrawElementsBaseVertex(GLenum mode,GLsizei count,GLenum type,const void* indices,GLint basevertex){
	printf("Unimplemented: glDrawElementsBaseVertex\n");
}



SYS_PUBLIC void APIENTRY glDrawElementsInstanced(GLenum mode,GLsizei count,GLenum type,const void* indices,GLsizei instancecount){
	printf("Unimplemented: glDrawElementsInstanced\n");
}



SYS_PUBLIC void APIENTRY glDrawElementsInstancedBaseVertex(GLenum mode,GLsizei count,GLenum type,const void* indices,GLsizei instancecount,GLint basevertex){
	printf("Unimplemented: glDrawElementsInstancedBaseVertex\n");
}



SYS_PUBLIC void APIENTRY glDrawRangeElements(GLenum mode,GLuint start,GLuint end,GLsizei count,GLenum type,const void* indices){
	printf("Unimplemented: glDrawRangeElements\n");
}



SYS_PUBLIC void APIENTRY glDrawRangeElementsBaseVertex(GLenum mode,GLuint start,GLuint end,GLsizei count,GLenum type,const void* indices,GLint basevertex){
	printf("Unimplemented: glDrawRangeElementsBaseVertex\n");
}



SYS_PUBLIC void APIENTRY glEnable(GLenum cap){
	printf("Unimplemented: glEnable\n");
}



SYS_PUBLIC void APIENTRY glEnablei(GLenum target,GLuint index){
	printf("Unimplemented: glEnablei\n");
}



SYS_PUBLIC void APIENTRY glEnableVertexAttribArray(GLuint index){
	printf("Unimplemented: glEnableVertexAttribArray\n");
}



SYS_PUBLIC void APIENTRY glEndConditionalRender(void){
	printf("Unimplemented: glEndConditionalRender\n");
}



SYS_PUBLIC void APIENTRY glEndQuery(GLenum target){
	printf("Unimplemented: glEndQuery\n");
}



SYS_PUBLIC void APIENTRY glEndTransformFeedback(void){
	printf("Unimplemented: glEndTransformFeedback\n");
}



SYS_PUBLIC void APIENTRY glFinish(void){
	printf("Unimplemented: glFinish\n");
}



SYS_PUBLIC void APIENTRY glFlush(void){
	printf("Unimplemented: glFlush\n");
}



SYS_PUBLIC void APIENTRY glFlushMappedBufferRange(GLenum target,GLintptr offset,GLsizeiptr length){
	printf("Unimplemented: glFlushMappedBufferRange\n");
}



SYS_PUBLIC void APIENTRY glFramebufferRenderbuffer(GLenum target,GLenum attachment,GLenum renderbuffertarget,GLuint renderbuffer){
	printf("Unimplemented: glFramebufferRenderbuffer\n");
}



SYS_PUBLIC void APIENTRY glFramebufferTexture(GLenum target,GLenum attachment,GLuint texture,GLint level){
	printf("Unimplemented: glFramebufferTexture\n");
}



SYS_PUBLIC void APIENTRY glFramebufferTexture1D(GLenum target,GLenum attachment,GLenum textarget,GLuint texture,GLint level){
	printf("Unimplemented: glFramebufferTexture1D\n");
}



SYS_PUBLIC void APIENTRY glFramebufferTexture2D(GLenum target,GLenum attachment,GLenum textarget,GLuint texture,GLint level){
	printf("Unimplemented: glFramebufferTexture2D\n");
}



SYS_PUBLIC void APIENTRY glFramebufferTexture3D(GLenum target,GLenum attachment,GLenum textarget,GLuint texture,GLint level,GLint zoffset){
	printf("Unimplemented: glFramebufferTexture3D\n");
}



SYS_PUBLIC void APIENTRY glFramebufferTextureLayer(GLenum target,GLenum attachment,GLuint texture,GLint level,GLint layer){
	printf("Unimplemented: glFramebufferTextureLayer\n");
}



SYS_PUBLIC void APIENTRY glFrontFace(GLenum mode){
	printf("Unimplemented: glFrontFace\n");
}



SYS_PUBLIC void APIENTRY glGenBuffers(GLsizei n,GLuint* buffers){
	printf("Unimplemented: glGenBuffers\n");
}



SYS_PUBLIC void APIENTRY glGenerateMipmap(GLenum target){
	printf("Unimplemented: glGenerateMipmap\n");
}



SYS_PUBLIC void APIENTRY glGenFramebuffers(GLsizei n,GLuint* framebuffers){
	printf("Unimplemented: glGenFramebuffers\n");
}



SYS_PUBLIC void APIENTRY glGenQueries(GLsizei n,GLuint* ids){
	printf("Unimplemented: glGenQueries\n");
}



SYS_PUBLIC void APIENTRY glGenRenderbuffers(GLsizei n,GLuint* renderbuffers){
	printf("Unimplemented: glGenRenderbuffers\n");
}



SYS_PUBLIC void APIENTRY glGenSamplers(GLsizei count,GLuint* samplers){
	printf("Unimplemented: glGenSamplers\n");
}



SYS_PUBLIC void APIENTRY glGenTextures(GLsizei n,GLuint* textures){
	printf("Unimplemented: glGenTextures\n");
}



SYS_PUBLIC void APIENTRY glGenVertexArrays(GLsizei n,GLuint* arrays){
	printf("Unimplemented: glGenVertexArrays\n");
}



SYS_PUBLIC void APIENTRY glGetActiveAttrib(GLuint program,GLuint index,GLsizei bufSize,GLsizei* length,GLint* size,GLenum* type,GLchar* name){
	printf("Unimplemented: glGetActiveAttrib\n");
}



SYS_PUBLIC void APIENTRY glGetActiveUniform(GLuint program,GLuint index,GLsizei bufSize,GLsizei* length,GLint* size,GLenum* type,GLchar* name){
	printf("Unimplemented: glGetActiveUniform\n");
}



SYS_PUBLIC void APIENTRY glGetActiveUniformBlockiv(GLuint program,GLuint uniformBlockIndex,GLenum pname,GLint* params){
	printf("Unimplemented: glGetActiveUniformBlockiv\n");
}



SYS_PUBLIC void APIENTRY glGetActiveUniformBlockName(GLuint program,GLuint uniformBlockIndex,GLsizei bufSize,GLsizei* length,GLchar* uniformBlockName){
	printf("Unimplemented: glGetActiveUniformBlockName\n");
}



SYS_PUBLIC void APIENTRY glGetActiveUniformName(GLuint program,GLuint uniformIndex,GLsizei bufSize,GLsizei* length,GLchar* uniformName){
	printf("Unimplemented: glGetActiveUniformName\n");
}



SYS_PUBLIC void APIENTRY glGetActiveUniformsiv(GLuint program,GLsizei uniformCount,const GLuint* uniformIndices,GLenum pname,GLint* params){
	printf("Unimplemented: glGetActiveUniformsiv\n");
}



SYS_PUBLIC void APIENTRY glGetAttachedShaders(GLuint program,GLsizei maxCount,GLsizei* count,GLuint* shaders){
	printf("Unimplemented: glGetAttachedShaders\n");
}



SYS_PUBLIC void APIENTRY glGetBooleani_v(GLenum target,GLuint index,GLboolean* data){
	printf("Unimplemented: glGetBooleani_v\n");
}



SYS_PUBLIC void APIENTRY glGetBooleanv(GLenum pname,GLboolean* data){
	printf("Unimplemented: glGetBooleanv\n");
}



SYS_PUBLIC void APIENTRY glGetBufferParameteri64v(GLenum target,GLenum pname,GLint64* params){
	printf("Unimplemented: glGetBufferParameteri64v\n");
}



SYS_PUBLIC void APIENTRY glGetBufferParameteriv(GLenum target,GLenum pname,GLint* params){
	printf("Unimplemented: glGetBufferParameteriv\n");
}



SYS_PUBLIC void APIENTRY glGetBufferPointerv(GLenum target,GLenum pname,void* *params){
	printf("Unimplemented: glGetBufferPointerv\n");
}



SYS_PUBLIC void APIENTRY glGetBufferSubData(GLenum target,GLintptr offset,GLsizeiptr size,void* data){
	printf("Unimplemented: glGetBufferSubData\n");
}



SYS_PUBLIC void APIENTRY glGetCompressedTexImage(GLenum target,GLint level,void* img){
	printf("Unimplemented: glGetCompressedTexImage\n");
}



SYS_PUBLIC void APIENTRY glGetDoublev(GLenum pname,GLdouble* data){
	printf("Unimplemented: glGetDoublev\n");
}



SYS_PUBLIC void APIENTRY glGetFloatv(GLenum pname,GLfloat* data){
	printf("Unimplemented: glGetFloatv\n");
}



SYS_PUBLIC void APIENTRY glGetFramebufferAttachmentParameteriv(GLenum target,GLenum attachment,GLenum pname,GLint* params){
	printf("Unimplemented: glGetFramebufferAttachmentParameteriv\n");
}



SYS_PUBLIC void APIENTRY glGetInteger64i_v(GLenum target,GLuint index,GLint64* data){
	printf("Unimplemented: glGetInteger64i_v\n");
}



SYS_PUBLIC void APIENTRY glGetInteger64v(GLenum pname,GLint64* data){
	printf("Unimplemented: glGetInteger64v\n");
}



SYS_PUBLIC void APIENTRY glGetIntegeri_v(GLenum target,GLuint index,GLint* data){
	printf("Unimplemented: glGetIntegeri_v\n");
}



SYS_PUBLIC void APIENTRY glGetIntegerv(GLenum pname,GLint* data){
	printf("Unimplemented: glGetIntegerv\n");
}



SYS_PUBLIC void APIENTRY glGetMultisamplefv(GLenum pname,GLuint index,GLfloat* val){
	printf("Unimplemented: glGetMultisamplefv\n");
}



SYS_PUBLIC void APIENTRY glGetPointerv(GLenum pname,void* *params){
	printf("Unimplemented: glGetPointerv\n");
}



SYS_PUBLIC void APIENTRY glGetProgramInfoLog(GLuint program,GLsizei bufSize,GLsizei* length,GLchar* infoLog){
	printf("Unimplemented: glGetProgramInfoLog\n");
}



SYS_PUBLIC void APIENTRY glGetProgramiv(GLuint program,GLenum pname,GLint* params){
	printf("Unimplemented: glGetProgramiv\n");
}



SYS_PUBLIC void APIENTRY glGetQueryiv(GLenum target,GLenum pname,GLint* params){
	printf("Unimplemented: glGetQueryiv\n");
}



SYS_PUBLIC void APIENTRY glGetQueryObjecti64v(GLuint id,GLenum pname,GLint64* params){
	printf("Unimplemented: glGetQueryObjecti64v\n");
}



SYS_PUBLIC void APIENTRY glGetQueryObjectiv(GLuint id,GLenum pname,GLint* params){
	printf("Unimplemented: glGetQueryObjectiv\n");
}



SYS_PUBLIC void APIENTRY glGetQueryObjectui64v(GLuint id,GLenum pname,GLuint64* params){
	printf("Unimplemented: glGetQueryObjectui64v\n");
}



SYS_PUBLIC void APIENTRY glGetQueryObjectuiv(GLuint id,GLenum pname,GLuint* params){
	printf("Unimplemented: glGetQueryObjectuiv\n");
}



SYS_PUBLIC void APIENTRY glGetRenderbufferParameteriv(GLenum target,GLenum pname,GLint* params){
	printf("Unimplemented: glGetRenderbufferParameteriv\n");
}



SYS_PUBLIC void APIENTRY glGetSamplerParameterfv(GLuint sampler,GLenum pname,GLfloat* params){
	printf("Unimplemented: glGetSamplerParameterfv\n");
}



SYS_PUBLIC void APIENTRY glGetSamplerParameterIiv(GLuint sampler,GLenum pname,GLint* params){
	printf("Unimplemented: glGetSamplerParameterIiv\n");
}



SYS_PUBLIC void APIENTRY glGetSamplerParameterIuiv(GLuint sampler,GLenum pname,GLuint* params){
	printf("Unimplemented: glGetSamplerParameterIuiv\n");
}



SYS_PUBLIC void APIENTRY glGetSamplerParameteriv(GLuint sampler,GLenum pname,GLint* params){
	printf("Unimplemented: glGetSamplerParameteriv\n");
}



SYS_PUBLIC void APIENTRY glGetShaderInfoLog(GLuint shader,GLsizei bufSize,GLsizei* length,GLchar* infoLog){
	printf("Unimplemented: glGetShaderInfoLog\n");
}



SYS_PUBLIC void APIENTRY glGetShaderiv(GLuint shader,GLenum pname,GLint* params){
	printf("Unimplemented: glGetShaderiv\n");
}



SYS_PUBLIC void APIENTRY glGetShaderSource(GLuint shader,GLsizei bufSize,GLsizei* length,GLchar* source){
	printf("Unimplemented: glGetShaderSource\n");
}



SYS_PUBLIC void APIENTRY glGetSynciv(GLsync sync,GLenum pname,GLsizei count,GLsizei* length,GLint* values){
	printf("Unimplemented: glGetSynciv\n");
}



SYS_PUBLIC void APIENTRY glGetTexImage(GLenum target,GLint level,GLenum format,GLenum type,void* pixels){
	printf("Unimplemented: glGetTexImage\n");
}



SYS_PUBLIC void APIENTRY glGetTexLevelParameterfv(GLenum target,GLint level,GLenum pname,GLfloat* params){
	printf("Unimplemented: glGetTexLevelParameterfv\n");
}



SYS_PUBLIC void APIENTRY glGetTexLevelParameteriv(GLenum target,GLint level,GLenum pname,GLint* params){
	printf("Unimplemented: glGetTexLevelParameteriv\n");
}



SYS_PUBLIC void APIENTRY glGetTexParameterfv(GLenum target,GLenum pname,GLfloat* params){
	printf("Unimplemented: glGetTexParameterfv\n");
}



SYS_PUBLIC void APIENTRY glGetTexParameterIiv(GLenum target,GLenum pname,GLint* params){
	printf("Unimplemented: glGetTexParameterIiv\n");
}



SYS_PUBLIC void APIENTRY glGetTexParameterIuiv(GLenum target,GLenum pname,GLuint* params){
	printf("Unimplemented: glGetTexParameterIuiv\n");
}



SYS_PUBLIC void APIENTRY glGetTexParameteriv(GLenum target,GLenum pname,GLint* params){
	printf("Unimplemented: glGetTexParameteriv\n");
}



SYS_PUBLIC void APIENTRY glGetTransformFeedbackVarying(GLuint program,GLuint index,GLsizei bufSize,GLsizei* length,GLsizei* size,GLenum* type,GLchar* name){
	printf("Unimplemented: glGetTransformFeedbackVarying\n");
}



SYS_PUBLIC void APIENTRY glGetUniformfv(GLuint program,GLint location,GLfloat* params){
	printf("Unimplemented: glGetUniformfv\n");
}



SYS_PUBLIC void APIENTRY glGetUniformIndices(GLuint program,GLsizei uniformCount,const GLchar* const*uniformNames,GLuint* uniformIndices){
	printf("Unimplemented: glGetUniformIndices\n");
}



SYS_PUBLIC void APIENTRY glGetUniformiv(GLuint program,GLint location,GLint* params){
	printf("Unimplemented: glGetUniformiv\n");
}



SYS_PUBLIC void APIENTRY glGetUniformuiv(GLuint program,GLint location,GLuint* params){
	printf("Unimplemented: glGetUniformuiv\n");
}



SYS_PUBLIC void APIENTRY glGetVertexAttribdv(GLuint index,GLenum pname,GLdouble* params){
	printf("Unimplemented: glGetVertexAttribdv\n");
}



SYS_PUBLIC void APIENTRY glGetVertexAttribfv(GLuint index,GLenum pname,GLfloat* params){
	printf("Unimplemented: glGetVertexAttribfv\n");
}



SYS_PUBLIC void APIENTRY glGetVertexAttribIiv(GLuint index,GLenum pname,GLint* params){
	printf("Unimplemented: glGetVertexAttribIiv\n");
}



SYS_PUBLIC void APIENTRY glGetVertexAttribIuiv(GLuint index,GLenum pname,GLuint* params){
	printf("Unimplemented: glGetVertexAttribIuiv\n");
}



SYS_PUBLIC void APIENTRY glGetVertexAttribiv(GLuint index,GLenum pname,GLint* params){
	printf("Unimplemented: glGetVertexAttribiv\n");
}



SYS_PUBLIC void APIENTRY glGetVertexAttribPointerv(GLuint index,GLenum pname,void* *pointer){
	printf("Unimplemented: glGetVertexAttribPointerv\n");
}



SYS_PUBLIC void APIENTRY glHint(GLenum target,GLenum mode){
	printf("Unimplemented: glHint\n");
}



SYS_PUBLIC void APIENTRY glLineWidth(GLfloat width){
	printf("Unimplemented: glLineWidth\n");
}



SYS_PUBLIC void APIENTRY glLinkProgram(GLuint program){
	printf("Unimplemented: glLinkProgram\n");
}



SYS_PUBLIC void APIENTRY glLogicOp(GLenum opcode){
	printf("Unimplemented: glLogicOp\n");
}



SYS_PUBLIC void APIENTRY glMultiDrawArrays(GLenum mode,const GLint* first,const GLsizei* count,GLsizei drawcount){
	printf("Unimplemented: glMultiDrawArrays\n");
}



SYS_PUBLIC void APIENTRY glMultiDrawElements(GLenum mode,const GLsizei* count,GLenum type,const void* const*indices,GLsizei drawcount){
	printf("Unimplemented: glMultiDrawElements\n");
}



SYS_PUBLIC void APIENTRY glMultiDrawElementsBaseVertex(GLenum mode,const GLsizei* count,GLenum type,const void* const*indices,GLsizei drawcount,const GLint* basevertex){
	printf("Unimplemented: glMultiDrawElementsBaseVertex\n");
}



SYS_PUBLIC void APIENTRY glPixelStoref(GLenum pname,GLfloat param){
	printf("Unimplemented: glPixelStoref\n");
}



SYS_PUBLIC void APIENTRY glPixelStorei(GLenum pname,GLint param){
	printf("Unimplemented: glPixelStorei\n");
}



SYS_PUBLIC void APIENTRY glPointParameterf(GLenum pname,GLfloat param){
	printf("Unimplemented: glPointParameterf\n");
}



SYS_PUBLIC void APIENTRY glPointParameterfv(GLenum pname,const GLfloat* params){
	printf("Unimplemented: glPointParameterfv\n");
}



SYS_PUBLIC void APIENTRY glPointParameteri(GLenum pname,GLint param){
	printf("Unimplemented: glPointParameteri\n");
}



SYS_PUBLIC void APIENTRY glPointParameteriv(GLenum pname,const GLint* params){
	printf("Unimplemented: glPointParameteriv\n");
}



SYS_PUBLIC void APIENTRY glPointSize(GLfloat size){
	printf("Unimplemented: glPointSize\n");
}



SYS_PUBLIC void APIENTRY glPolygonMode(GLenum face,GLenum mode){
	printf("Unimplemented: glPolygonMode\n");
}



SYS_PUBLIC void APIENTRY glPolygonOffset(GLfloat factor,GLfloat units){
	printf("Unimplemented: glPolygonOffset\n");
}



SYS_PUBLIC void APIENTRY glPrimitiveRestartIndex(GLuint index){
	printf("Unimplemented: glPrimitiveRestartIndex\n");
}



SYS_PUBLIC void APIENTRY glProvokingVertex(GLenum mode){
	printf("Unimplemented: glProvokingVertex\n");
}



SYS_PUBLIC void APIENTRY glQueryCounter(GLuint id,GLenum target){
	printf("Unimplemented: glQueryCounter\n");
}



SYS_PUBLIC void APIENTRY glReadBuffer(GLenum src){
	printf("Unimplemented: glReadBuffer\n");
}



SYS_PUBLIC void APIENTRY glReadPixels(GLint x,GLint y,GLsizei width,GLsizei height,GLenum format,GLenum type,void* pixels){
	printf("Unimplemented: glReadPixels\n");
}



SYS_PUBLIC void APIENTRY glRenderbufferStorage(GLenum target,GLenum internalformat,GLsizei width,GLsizei height){
	printf("Unimplemented: glRenderbufferStorage\n");
}



SYS_PUBLIC void APIENTRY glRenderbufferStorageMultisample(GLenum target,GLsizei samples,GLenum internalformat,GLsizei width,GLsizei height){
	printf("Unimplemented: glRenderbufferStorageMultisample\n");
}



SYS_PUBLIC void APIENTRY glSampleCoverage(GLfloat value,GLboolean invert){
	printf("Unimplemented: glSampleCoverage\n");
}



SYS_PUBLIC void APIENTRY glSampleMaski(GLuint maskNumber,GLbitfield mask){
	printf("Unimplemented: glSampleMaski\n");
}



SYS_PUBLIC void APIENTRY glSamplerParameterf(GLuint sampler,GLenum pname,GLfloat param){
	printf("Unimplemented: glSamplerParameterf\n");
}



SYS_PUBLIC void APIENTRY glSamplerParameterfv(GLuint sampler,GLenum pname,const GLfloat* param){
	printf("Unimplemented: glSamplerParameterfv\n");
}



SYS_PUBLIC void APIENTRY glSamplerParameteri(GLuint sampler,GLenum pname,GLint param){
	printf("Unimplemented: glSamplerParameteri\n");
}



SYS_PUBLIC void APIENTRY glSamplerParameterIiv(GLuint sampler,GLenum pname,const GLint* param){
	printf("Unimplemented: glSamplerParameterIiv\n");
}



SYS_PUBLIC void APIENTRY glSamplerParameterIuiv(GLuint sampler,GLenum pname,const GLuint* param){
	printf("Unimplemented: glSamplerParameterIuiv\n");
}



SYS_PUBLIC void APIENTRY glSamplerParameteriv(GLuint sampler,GLenum pname,const GLint* param){
	printf("Unimplemented: glSamplerParameteriv\n");
}



SYS_PUBLIC void APIENTRY glScissor(GLint x,GLint y,GLsizei width,GLsizei height){
	printf("Unimplemented: glScissor\n");
}



SYS_PUBLIC void APIENTRY glShaderSource(GLuint shader,GLsizei count,const GLchar* const*string,const GLint* length){
	printf("Unimplemented: glShaderSource\n");
}



SYS_PUBLIC void APIENTRY glStencilFunc(GLenum func,GLint ref,GLuint mask){
	printf("Unimplemented: glStencilFunc\n");
}



SYS_PUBLIC void APIENTRY glStencilFuncSeparate(GLenum face,GLenum func,GLint ref,GLuint mask){
	printf("Unimplemented: glStencilFuncSeparate\n");
}



SYS_PUBLIC void APIENTRY glStencilMask(GLuint mask){
	printf("Unimplemented: glStencilMask\n");
}



SYS_PUBLIC void APIENTRY glStencilMaskSeparate(GLenum face,GLuint mask){
	printf("Unimplemented: glStencilMaskSeparate\n");
}



SYS_PUBLIC void APIENTRY glStencilOp(GLenum fail,GLenum zfail,GLenum zpass){
	printf("Unimplemented: glStencilOp\n");
}



SYS_PUBLIC void APIENTRY glStencilOpSeparate(GLenum face,GLenum sfail,GLenum dpfail,GLenum dppass){
	printf("Unimplemented: glStencilOpSeparate\n");
}



SYS_PUBLIC void APIENTRY glTexBuffer(GLenum target,GLenum internalformat,GLuint buffer){
	printf("Unimplemented: glTexBuffer\n");
}



SYS_PUBLIC void APIENTRY glTexImage1D(GLenum target,GLint level,GLint internalformat,GLsizei width,GLint border,GLenum format,GLenum type,const void* pixels){
	printf("Unimplemented: glTexImage1D\n");
}



SYS_PUBLIC void APIENTRY glTexImage2D(GLenum target,GLint level,GLint internalformat,GLsizei width,GLsizei height,GLint border,GLenum format,GLenum type,const void* pixels){
	printf("Unimplemented: glTexImage2D\n");
}



SYS_PUBLIC void APIENTRY glTexImage2DMultisample(GLenum target,GLsizei samples,GLenum internalformat,GLsizei width,GLsizei height,GLboolean fixedsamplelocations){
	printf("Unimplemented: glTexImage2DMultisample\n");
}



SYS_PUBLIC void APIENTRY glTexImage3D(GLenum target,GLint level,GLint internalformat,GLsizei width,GLsizei height,GLsizei depth,GLint border,GLenum format,GLenum type,const void* pixels){
	printf("Unimplemented: glTexImage3D\n");
}



SYS_PUBLIC void APIENTRY glTexImage3DMultisample(GLenum target,GLsizei samples,GLenum internalformat,GLsizei width,GLsizei height,GLsizei depth,GLboolean fixedsamplelocations){
	printf("Unimplemented: glTexImage3DMultisample\n");
}



SYS_PUBLIC void APIENTRY glTexParameterf(GLenum target,GLenum pname,GLfloat param){
	printf("Unimplemented: glTexParameterf\n");
}



SYS_PUBLIC void APIENTRY glTexParameterfv(GLenum target,GLenum pname,const GLfloat* params){
	printf("Unimplemented: glTexParameterfv\n");
}



SYS_PUBLIC void APIENTRY glTexParameteri(GLenum target,GLenum pname,GLint param){
	printf("Unimplemented: glTexParameteri\n");
}



SYS_PUBLIC void APIENTRY glTexParameterIiv(GLenum target,GLenum pname,const GLint* params){
	printf("Unimplemented: glTexParameterIiv\n");
}



SYS_PUBLIC void APIENTRY glTexParameterIuiv(GLenum target,GLenum pname,const GLuint* params){
	printf("Unimplemented: glTexParameterIuiv\n");
}



SYS_PUBLIC void APIENTRY glTexParameteriv(GLenum target,GLenum pname,const GLint* params){
	printf("Unimplemented: glTexParameteriv\n");
}



SYS_PUBLIC void APIENTRY glTexSubImage1D(GLenum target,GLint level,GLint xoffset,GLsizei width,GLenum format,GLenum type,const void* pixels){
	printf("Unimplemented: glTexSubImage1D\n");
}



SYS_PUBLIC void APIENTRY glTexSubImage2D(GLenum target,GLint level,GLint xoffset,GLint yoffset,GLsizei width,GLsizei height,GLenum format,GLenum type,const void* pixels){
	printf("Unimplemented: glTexSubImage2D\n");
}



SYS_PUBLIC void APIENTRY glTexSubImage3D(GLenum target,GLint level,GLint xoffset,GLint yoffset,GLint zoffset,GLsizei width,GLsizei height,GLsizei depth,GLenum format,GLenum type,const void* pixels){
	printf("Unimplemented: glTexSubImage3D\n");
}



SYS_PUBLIC void APIENTRY glTransformFeedbackVaryings(GLuint program,GLsizei count,const GLchar* const*varyings,GLenum bufferMode){
	printf("Unimplemented: glTransformFeedbackVaryings\n");
}



SYS_PUBLIC void APIENTRY glUniform1f(GLint location,GLfloat v0){
	printf("Unimplemented: glUniform1f\n");
}



SYS_PUBLIC void APIENTRY glUniform1fv(GLint location,GLsizei count,const GLfloat* value){
	printf("Unimplemented: glUniform1fv\n");
}



SYS_PUBLIC void APIENTRY glUniform1i(GLint location,GLint v0){
	printf("Unimplemented: glUniform1i\n");
}



SYS_PUBLIC void APIENTRY glUniform1iv(GLint location,GLsizei count,const GLint* value){
	printf("Unimplemented: glUniform1iv\n");
}



SYS_PUBLIC void APIENTRY glUniform1ui(GLint location,GLuint v0){
	printf("Unimplemented: glUniform1ui\n");
}



SYS_PUBLIC void APIENTRY glUniform1uiv(GLint location,GLsizei count,const GLuint* value){
	printf("Unimplemented: glUniform1uiv\n");
}



SYS_PUBLIC void APIENTRY glUniform2f(GLint location,GLfloat v0,GLfloat v1){
	printf("Unimplemented: glUniform2f\n");
}



SYS_PUBLIC void APIENTRY glUniform2fv(GLint location,GLsizei count,const GLfloat* value){
	printf("Unimplemented: glUniform2fv\n");
}



SYS_PUBLIC void APIENTRY glUniform2i(GLint location,GLint v0,GLint v1){
	printf("Unimplemented: glUniform2i\n");
}



SYS_PUBLIC void APIENTRY glUniform2iv(GLint location,GLsizei count,const GLint* value){
	printf("Unimplemented: glUniform2iv\n");
}



SYS_PUBLIC void APIENTRY glUniform2ui(GLint location,GLuint v0,GLuint v1){
	printf("Unimplemented: glUniform2ui\n");
}



SYS_PUBLIC void APIENTRY glUniform2uiv(GLint location,GLsizei count,const GLuint* value){
	printf("Unimplemented: glUniform2uiv\n");
}



SYS_PUBLIC void APIENTRY glUniform3f(GLint location,GLfloat v0,GLfloat v1,GLfloat v2){
	printf("Unimplemented: glUniform3f\n");
}



SYS_PUBLIC void APIENTRY glUniform3fv(GLint location,GLsizei count,const GLfloat* value){
	printf("Unimplemented: glUniform3fv\n");
}



SYS_PUBLIC void APIENTRY glUniform3i(GLint location,GLint v0,GLint v1,GLint v2){
	printf("Unimplemented: glUniform3i\n");
}



SYS_PUBLIC void APIENTRY glUniform3iv(GLint location,GLsizei count,const GLint* value){
	printf("Unimplemented: glUniform3iv\n");
}



SYS_PUBLIC void APIENTRY glUniform3ui(GLint location,GLuint v0,GLuint v1,GLuint v2){
	printf("Unimplemented: glUniform3ui\n");
}



SYS_PUBLIC void APIENTRY glUniform3uiv(GLint location,GLsizei count,const GLuint* value){
	printf("Unimplemented: glUniform3uiv\n");
}



SYS_PUBLIC void APIENTRY glUniform4f(GLint location,GLfloat v0,GLfloat v1,GLfloat v2,GLfloat v3){
	printf("Unimplemented: glUniform4f\n");
}



SYS_PUBLIC void APIENTRY glUniform4fv(GLint location,GLsizei count,const GLfloat* value){
	printf("Unimplemented: glUniform4fv\n");
}



SYS_PUBLIC void APIENTRY glUniform4i(GLint location,GLint v0,GLint v1,GLint v2,GLint v3){
	printf("Unimplemented: glUniform4i\n");
}



SYS_PUBLIC void APIENTRY glUniform4iv(GLint location,GLsizei count,const GLint* value){
	printf("Unimplemented: glUniform4iv\n");
}



SYS_PUBLIC void APIENTRY glUniform4ui(GLint location,GLuint v0,GLuint v1,GLuint v2,GLuint v3){
	printf("Unimplemented: glUniform4ui\n");
}



SYS_PUBLIC void APIENTRY glUniform4uiv(GLint location,GLsizei count,const GLuint* value){
	printf("Unimplemented: glUniform4uiv\n");
}



SYS_PUBLIC void APIENTRY glUniformBlockBinding(GLuint program,GLuint uniformBlockIndex,GLuint uniformBlockBinding){
	printf("Unimplemented: glUniformBlockBinding\n");
}



SYS_PUBLIC void APIENTRY glUniformMatrix2fv(GLint location,GLsizei count,GLboolean transpose,const GLfloat* value){
	printf("Unimplemented: glUniformMatrix2fv\n");
}



SYS_PUBLIC void APIENTRY glUniformMatrix2x3fv(GLint location,GLsizei count,GLboolean transpose,const GLfloat* value){
	printf("Unimplemented: glUniformMatrix2x3fv\n");
}



SYS_PUBLIC void APIENTRY glUniformMatrix2x4fv(GLint location,GLsizei count,GLboolean transpose,const GLfloat* value){
	printf("Unimplemented: glUniformMatrix2x4fv\n");
}



SYS_PUBLIC void APIENTRY glUniformMatrix3fv(GLint location,GLsizei count,GLboolean transpose,const GLfloat* value){
	printf("Unimplemented: glUniformMatrix3fv\n");
}



SYS_PUBLIC void APIENTRY glUniformMatrix3x2fv(GLint location,GLsizei count,GLboolean transpose,const GLfloat* value){
	printf("Unimplemented: glUniformMatrix3x2fv\n");
}



SYS_PUBLIC void APIENTRY glUniformMatrix3x4fv(GLint location,GLsizei count,GLboolean transpose,const GLfloat* value){
	printf("Unimplemented: glUniformMatrix3x4fv\n");
}



SYS_PUBLIC void APIENTRY glUniformMatrix4fv(GLint location,GLsizei count,GLboolean transpose,const GLfloat* value){
	printf("Unimplemented: glUniformMatrix4fv\n");
}



SYS_PUBLIC void APIENTRY glUniformMatrix4x2fv(GLint location,GLsizei count,GLboolean transpose,const GLfloat* value){
	printf("Unimplemented: glUniformMatrix4x2fv\n");
}



SYS_PUBLIC void APIENTRY glUniformMatrix4x3fv(GLint location,GLsizei count,GLboolean transpose,const GLfloat* value){
	printf("Unimplemented: glUniformMatrix4x3fv\n");
}



SYS_PUBLIC void APIENTRY glUseProgram(GLuint program){
	printf("Unimplemented: glUseProgram\n");
}



SYS_PUBLIC void APIENTRY glValidateProgram(GLuint program){
	printf("Unimplemented: glValidateProgram\n");
}



SYS_PUBLIC void APIENTRY glVertexAttrib1d(GLuint index,GLdouble x){
	printf("Unimplemented: glVertexAttrib1d\n");
}



SYS_PUBLIC void APIENTRY glVertexAttrib1dv(GLuint index,const GLdouble* v){
	printf("Unimplemented: glVertexAttrib1dv\n");
}



SYS_PUBLIC void APIENTRY glVertexAttrib1f(GLuint index,GLfloat x){
	printf("Unimplemented: glVertexAttrib1f\n");
}



SYS_PUBLIC void APIENTRY glVertexAttrib1fv(GLuint index,const GLfloat* v){
	printf("Unimplemented: glVertexAttrib1fv\n");
}



SYS_PUBLIC void APIENTRY glVertexAttrib1s(GLuint index,GLshort x){
	printf("Unimplemented: glVertexAttrib1s\n");
}



SYS_PUBLIC void APIENTRY glVertexAttrib1sv(GLuint index,const GLshort* v){
	printf("Unimplemented: glVertexAttrib1sv\n");
}



SYS_PUBLIC void APIENTRY glVertexAttrib2d(GLuint index,GLdouble x,GLdouble y){
	printf("Unimplemented: glVertexAttrib2d\n");
}



SYS_PUBLIC void APIENTRY glVertexAttrib2dv(GLuint index,const GLdouble* v){
	printf("Unimplemented: glVertexAttrib2dv\n");
}



SYS_PUBLIC void APIENTRY glVertexAttrib2f(GLuint index,GLfloat x,GLfloat y){
	printf("Unimplemented: glVertexAttrib2f\n");
}



SYS_PUBLIC void APIENTRY glVertexAttrib2fv(GLuint index,const GLfloat* v){
	printf("Unimplemented: glVertexAttrib2fv\n");
}



SYS_PUBLIC void APIENTRY glVertexAttrib2s(GLuint index,GLshort x,GLshort y){
	printf("Unimplemented: glVertexAttrib2s\n");
}



SYS_PUBLIC void APIENTRY glVertexAttrib2sv(GLuint index,const GLshort* v){
	printf("Unimplemented: glVertexAttrib2sv\n");
}



SYS_PUBLIC void APIENTRY glVertexAttrib3d(GLuint index,GLdouble x,GLdouble y,GLdouble z){
	printf("Unimplemented: glVertexAttrib3d\n");
}



SYS_PUBLIC void APIENTRY glVertexAttrib3dv(GLuint index,const GLdouble* v){
	printf("Unimplemented: glVertexAttrib3dv\n");
}



SYS_PUBLIC void APIENTRY glVertexAttrib3f(GLuint index,GLfloat x,GLfloat y,GLfloat z){
	printf("Unimplemented: glVertexAttrib3f\n");
}



SYS_PUBLIC void APIENTRY glVertexAttrib3fv(GLuint index,const GLfloat* v){
	printf("Unimplemented: glVertexAttrib3fv\n");
}



SYS_PUBLIC void APIENTRY glVertexAttrib3s(GLuint index,GLshort x,GLshort y,GLshort z){
	printf("Unimplemented: glVertexAttrib3s\n");
}



SYS_PUBLIC void APIENTRY glVertexAttrib3sv(GLuint index,const GLshort* v){
	printf("Unimplemented: glVertexAttrib3sv\n");
}



SYS_PUBLIC void APIENTRY glVertexAttrib4bv(GLuint index,const GLbyte* v){
	printf("Unimplemented: glVertexAttrib4bv\n");
}



SYS_PUBLIC void APIENTRY glVertexAttrib4d(GLuint index,GLdouble x,GLdouble y,GLdouble z,GLdouble w){
	printf("Unimplemented: glVertexAttrib4d\n");
}



SYS_PUBLIC void APIENTRY glVertexAttrib4dv(GLuint index,const GLdouble* v){
	printf("Unimplemented: glVertexAttrib4dv\n");
}



SYS_PUBLIC void APIENTRY glVertexAttrib4f(GLuint index,GLfloat x,GLfloat y,GLfloat z,GLfloat w){
	printf("Unimplemented: glVertexAttrib4f\n");
}



SYS_PUBLIC void APIENTRY glVertexAttrib4fv(GLuint index,const GLfloat* v){
	printf("Unimplemented: glVertexAttrib4fv\n");
}



SYS_PUBLIC void APIENTRY glVertexAttrib4iv(GLuint index,const GLint* v){
	printf("Unimplemented: glVertexAttrib4iv\n");
}



SYS_PUBLIC void APIENTRY glVertexAttrib4Nbv(GLuint index,const GLbyte* v){
	printf("Unimplemented: glVertexAttrib4Nbv\n");
}



SYS_PUBLIC void APIENTRY glVertexAttrib4Niv(GLuint index,const GLint* v){
	printf("Unimplemented: glVertexAttrib4Niv\n");
}



SYS_PUBLIC void APIENTRY glVertexAttrib4Nsv(GLuint index,const GLshort* v){
	printf("Unimplemented: glVertexAttrib4Nsv\n");
}



SYS_PUBLIC void APIENTRY glVertexAttrib4Nub(GLuint index,GLubyte x,GLubyte y,GLubyte z,GLubyte w){
	printf("Unimplemented: glVertexAttrib4Nub\n");
}



SYS_PUBLIC void APIENTRY glVertexAttrib4Nubv(GLuint index,const GLubyte* v){
	printf("Unimplemented: glVertexAttrib4Nubv\n");
}



SYS_PUBLIC void APIENTRY glVertexAttrib4Nuiv(GLuint index,const GLuint* v){
	printf("Unimplemented: glVertexAttrib4Nuiv\n");
}



SYS_PUBLIC void APIENTRY glVertexAttrib4Nusv(GLuint index,const GLushort* v){
	printf("Unimplemented: glVertexAttrib4Nusv\n");
}



SYS_PUBLIC void APIENTRY glVertexAttrib4s(GLuint index,GLshort x,GLshort y,GLshort z,GLshort w){
	printf("Unimplemented: glVertexAttrib4s\n");
}



SYS_PUBLIC void APIENTRY glVertexAttrib4sv(GLuint index,const GLshort* v){
	printf("Unimplemented: glVertexAttrib4sv\n");
}



SYS_PUBLIC void APIENTRY glVertexAttrib4ubv(GLuint index,const GLubyte* v){
	printf("Unimplemented: glVertexAttrib4ubv\n");
}



SYS_PUBLIC void APIENTRY glVertexAttrib4uiv(GLuint index,const GLuint* v){
	printf("Unimplemented: glVertexAttrib4uiv\n");
}



SYS_PUBLIC void APIENTRY glVertexAttrib4usv(GLuint index,const GLushort* v){
	printf("Unimplemented: glVertexAttrib4usv\n");
}



SYS_PUBLIC void APIENTRY glVertexAttribDivisor(GLuint index,GLuint divisor){
	printf("Unimplemented: glVertexAttribDivisor\n");
}



SYS_PUBLIC void APIENTRY glVertexAttribI1i(GLuint index,GLint x){
	printf("Unimplemented: glVertexAttribI1i\n");
}



SYS_PUBLIC void APIENTRY glVertexAttribI1iv(GLuint index,const GLint* v){
	printf("Unimplemented: glVertexAttribI1iv\n");
}



SYS_PUBLIC void APIENTRY glVertexAttribI1ui(GLuint index,GLuint x){
	printf("Unimplemented: glVertexAttribI1ui\n");
}



SYS_PUBLIC void APIENTRY glVertexAttribI1uiv(GLuint index,const GLuint* v){
	printf("Unimplemented: glVertexAttribI1uiv\n");
}



SYS_PUBLIC void APIENTRY glVertexAttribI2i(GLuint index,GLint x,GLint y){
	printf("Unimplemented: glVertexAttribI2i\n");
}



SYS_PUBLIC void APIENTRY glVertexAttribI2iv(GLuint index,const GLint* v){
	printf("Unimplemented: glVertexAttribI2iv\n");
}



SYS_PUBLIC void APIENTRY glVertexAttribI2ui(GLuint index,GLuint x,GLuint y){
	printf("Unimplemented: glVertexAttribI2ui\n");
}



SYS_PUBLIC void APIENTRY glVertexAttribI2uiv(GLuint index,const GLuint* v){
	printf("Unimplemented: glVertexAttribI2uiv\n");
}



SYS_PUBLIC void APIENTRY glVertexAttribI3i(GLuint index,GLint x,GLint y,GLint z){
	printf("Unimplemented: glVertexAttribI3i\n");
}



SYS_PUBLIC void APIENTRY glVertexAttribI3iv(GLuint index,const GLint* v){
	printf("Unimplemented: glVertexAttribI3iv\n");
}



SYS_PUBLIC void APIENTRY glVertexAttribI3ui(GLuint index,GLuint x,GLuint y,GLuint z){
	printf("Unimplemented: glVertexAttribI3ui\n");
}



SYS_PUBLIC void APIENTRY glVertexAttribI3uiv(GLuint index,const GLuint* v){
	printf("Unimplemented: glVertexAttribI3uiv\n");
}



SYS_PUBLIC void APIENTRY glVertexAttribI4bv(GLuint index,const GLbyte* v){
	printf("Unimplemented: glVertexAttribI4bv\n");
}



SYS_PUBLIC void APIENTRY glVertexAttribI4i(GLuint index,GLint x,GLint y,GLint z,GLint w){
	printf("Unimplemented: glVertexAttribI4i\n");
}



SYS_PUBLIC void APIENTRY glVertexAttribI4iv(GLuint index,const GLint* v){
	printf("Unimplemented: glVertexAttribI4iv\n");
}



SYS_PUBLIC void APIENTRY glVertexAttribI4sv(GLuint index,const GLshort* v){
	printf("Unimplemented: glVertexAttribI4sv\n");
}



SYS_PUBLIC void APIENTRY glVertexAttribI4ubv(GLuint index,const GLubyte* v){
	printf("Unimplemented: glVertexAttribI4ubv\n");
}



SYS_PUBLIC void APIENTRY glVertexAttribI4ui(GLuint index,GLuint x,GLuint y,GLuint z,GLuint w){
	printf("Unimplemented: glVertexAttribI4ui\n");
}



SYS_PUBLIC void APIENTRY glVertexAttribI4uiv(GLuint index,const GLuint* v){
	printf("Unimplemented: glVertexAttribI4uiv\n");
}



SYS_PUBLIC void APIENTRY glVertexAttribI4usv(GLuint index,const GLushort* v){
	printf("Unimplemented: glVertexAttribI4usv\n");
}



SYS_PUBLIC void APIENTRY glVertexAttribIPointer(GLuint index,GLint size,GLenum type,GLsizei stride,const void* pointer){
	printf("Unimplemented: glVertexAttribIPointer\n");
}



SYS_PUBLIC void APIENTRY glVertexAttribP1ui(GLuint index,GLenum type,GLboolean normalized,GLuint value){
	printf("Unimplemented: glVertexAttribP1ui\n");
}



SYS_PUBLIC void APIENTRY glVertexAttribP1uiv(GLuint index,GLenum type,GLboolean normalized,const GLuint* value){
	printf("Unimplemented: glVertexAttribP1uiv\n");
}



SYS_PUBLIC void APIENTRY glVertexAttribP2ui(GLuint index,GLenum type,GLboolean normalized,GLuint value){
	printf("Unimplemented: glVertexAttribP2ui\n");
}



SYS_PUBLIC void APIENTRY glVertexAttribP2uiv(GLuint index,GLenum type,GLboolean normalized,const GLuint* value){
	printf("Unimplemented: glVertexAttribP2uiv\n");
}



SYS_PUBLIC void APIENTRY glVertexAttribP3ui(GLuint index,GLenum type,GLboolean normalized,GLuint value){
	printf("Unimplemented: glVertexAttribP3ui\n");
}



SYS_PUBLIC void APIENTRY glVertexAttribP3uiv(GLuint index,GLenum type,GLboolean normalized,const GLuint* value){
	printf("Unimplemented: glVertexAttribP3uiv\n");
}



SYS_PUBLIC void APIENTRY glVertexAttribP4ui(GLuint index,GLenum type,GLboolean normalized,GLuint value){
	printf("Unimplemented: glVertexAttribP4ui\n");
}



SYS_PUBLIC void APIENTRY glVertexAttribP4uiv(GLuint index,GLenum type,GLboolean normalized,const GLuint* value){
	printf("Unimplemented: glVertexAttribP4uiv\n");
}



SYS_PUBLIC void APIENTRY glVertexAttribPointer(GLuint index,GLint size,GLenum type,GLboolean normalized,GLsizei stride,const void* pointer){
	printf("Unimplemented: glVertexAttribPointer\n");
}



SYS_PUBLIC void APIENTRY glViewport(GLint x,GLint y,GLsizei width,GLsizei height){
	printf("Unimplemented: glViewport\n");
}



SYS_PUBLIC void APIENTRY glWaitSync(GLsync sync,GLbitfield flags,GLuint64 timeout){
	printf("Unimplemented: glWaitSync\n");
}
