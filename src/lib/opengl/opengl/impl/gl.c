#include <GL/gl.h>
#include <sys/io.h>



SYS_PUBLIC void glActiveTexture(GLenum texture){
	printf("Unimplemented: glActiveTexture\n");
}



SYS_PUBLIC void glAttachShader(GLuint program,GLuint shader){
	printf("Unimplemented: glAttachShader\n");
}



SYS_PUBLIC void glBeginConditionalRender(GLuint id,GLenum mode){
	printf("Unimplemented: glBeginConditionalRender\n");
}



SYS_PUBLIC void glBeginQuery(GLenum target,GLuint id){
	printf("Unimplemented: glBeginQuery\n");
}



SYS_PUBLIC void glBeginTransformFeedback(GLenum primitiveMode){
	printf("Unimplemented: glBeginTransformFeedback\n");
}



SYS_PUBLIC void glBindAttribLocation(GLuint program,GLuint index,const GLchar* name){
	printf("Unimplemented: glBindAttribLocation\n");
}



SYS_PUBLIC void glBindBuffer(GLenum target,GLuint buffer){
	printf("Unimplemented: glBindBuffer\n");
}



SYS_PUBLIC void glBindBufferBase(GLenum target,GLuint index,GLuint buffer){
	printf("Unimplemented: glBindBufferBase\n");
}



SYS_PUBLIC void glBindBufferRange(GLenum target,GLuint index,GLuint buffer,GLintptr offset,GLsizeiptr size){
	printf("Unimplemented: glBindBufferRange\n");
}



SYS_PUBLIC void glBindFragDataLocation(GLuint program,GLuint color,const GLchar* name){
	printf("Unimplemented: glBindFragDataLocation\n");
}



SYS_PUBLIC void glBindFragDataLocationIndexed(GLuint program,GLuint colorNumber,GLuint index,const GLchar* name){
	printf("Unimplemented: glBindFragDataLocationIndexed\n");
}



SYS_PUBLIC void glBindFramebuffer(GLenum target,GLuint framebuffer){
	printf("Unimplemented: glBindFramebuffer\n");
}



SYS_PUBLIC void glBindRenderbuffer(GLenum target,GLuint renderbuffer){
	printf("Unimplemented: glBindRenderbuffer\n");
}



SYS_PUBLIC void glBindSampler(GLuint unit,GLuint sampler){
	printf("Unimplemented: glBindSampler\n");
}



SYS_PUBLIC void glBindTexture(GLenum target,GLuint texture){
	printf("Unimplemented: glBindTexture\n");
}



SYS_PUBLIC void glBindVertexArray(GLuint array){
	printf("Unimplemented: glBindVertexArray\n");
}



SYS_PUBLIC void glBlendColor(GLfloat red,GLfloat green,GLfloat blue,GLfloat alpha){
	printf("Unimplemented: glBlendColor\n");
}



SYS_PUBLIC void glBlendEquation(GLenum mode){
	printf("Unimplemented: glBlendEquation\n");
}



SYS_PUBLIC void glBlendEquationSeparate(GLenum modeRGB,GLenum modeAlpha){
	printf("Unimplemented: glBlendEquationSeparate\n");
}



SYS_PUBLIC void glBlendFunc(GLenum sfactor,GLenum dfactor){
	printf("Unimplemented: glBlendFunc\n");
}



SYS_PUBLIC void glBlendFuncSeparate(GLenum sfactorRGB,GLenum dfactorRGB,GLenum sfactorAlpha,GLenum dfactorAlpha){
	printf("Unimplemented: glBlendFuncSeparate\n");
}



SYS_PUBLIC void glBlitFramebuffer(GLint srcX0,GLint srcY0,GLint srcX1,GLint srcY1,GLint dstX0,GLint dstY0,GLint dstX1,GLint dstY1,GLbitfield mask,GLenum filter){
	printf("Unimplemented: glBlitFramebuffer\n");
}



SYS_PUBLIC void glBufferData(GLenum target,GLsizeiptr size,const void* data,GLenum usage){
	printf("Unimplemented: glBufferData\n");
}



SYS_PUBLIC void glBufferSubData(GLenum target,GLintptr offset,GLsizeiptr size,const void* data){
	printf("Unimplemented: glBufferSubData\n");
}



SYS_PUBLIC GLenum glCheckFramebufferStatus(GLenum target){
	printf("Unimplemented: glCheckFramebufferStatus\n");
	return 0;
}



SYS_PUBLIC void glClampColor(GLenum target,GLenum clamp){
	printf("Unimplemented: glClampColor\n");
}



SYS_PUBLIC void glClear(GLbitfield mask){
	printf("Unimplemented: glClear\n");
}



SYS_PUBLIC void glClearBufferfi(GLenum buffer,GLint drawbuffer,GLfloat depth,GLint stencil){
	printf("Unimplemented: glClearBufferfi\n");
}



SYS_PUBLIC void glClearBufferfv(GLenum buffer,GLint drawbuffer,const GLfloat* value){
	printf("Unimplemented: glClearBufferfv\n");
}



SYS_PUBLIC void glClearBufferiv(GLenum buffer,GLint drawbuffer,const GLint* value){
	printf("Unimplemented: glClearBufferiv\n");
}



SYS_PUBLIC void glClearBufferuiv(GLenum buffer,GLint drawbuffer,const GLuint* value){
	printf("Unimplemented: glClearBufferuiv\n");
}



SYS_PUBLIC void glClearColor(GLfloat red,GLfloat green,GLfloat blue,GLfloat alpha){
	printf("Unimplemented: glClearColor\n");
}



SYS_PUBLIC void glClearDepth(GLdouble depth){
	printf("Unimplemented: glClearDepth\n");
}



SYS_PUBLIC void glClearStencil(GLint s){
	printf("Unimplemented: glClearStencil\n");
}



SYS_PUBLIC GLenum glClientWaitSync(GLsync sync,GLbitfield flags,GLuint64 timeout){
	printf("Unimplemented: glClientWaitSync\n");
	return 0;
}



SYS_PUBLIC void glColorMask(GLboolean red,GLboolean green,GLboolean blue,GLboolean alpha){
	printf("Unimplemented: glColorMask\n");
}



SYS_PUBLIC void glColorMaski(GLuint index,GLboolean r,GLboolean g,GLboolean b,GLboolean a){
	printf("Unimplemented: glColorMaski\n");
}



SYS_PUBLIC void glCompileShader(GLuint shader){
	printf("Unimplemented: glCompileShader\n");
}



SYS_PUBLIC void glCompressedTexImage1D(GLenum target,GLint level,GLenum internalformat,GLsizei width,GLint border,GLsizei imageSize,const void* data){
	printf("Unimplemented: glCompressedTexImage1D\n");
}



SYS_PUBLIC void glCompressedTexImage2D(GLenum target,GLint level,GLenum internalformat,GLsizei width,GLsizei height,GLint border,GLsizei imageSize,const void* data){
	printf("Unimplemented: glCompressedTexImage2D\n");
}



SYS_PUBLIC void glCompressedTexImage3D(GLenum target,GLint level,GLenum internalformat,GLsizei width,GLsizei height,GLsizei depth,GLint border,GLsizei imageSize,const void* data){
	printf("Unimplemented: glCompressedTexImage3D\n");
}



SYS_PUBLIC void glCompressedTexSubImage1D(GLenum target,GLint level,GLint xoffset,GLsizei width,GLenum format,GLsizei imageSize,const void* data){
	printf("Unimplemented: glCompressedTexSubImage1D\n");
}



SYS_PUBLIC void glCompressedTexSubImage2D(GLenum target,GLint level,GLint xoffset,GLint yoffset,GLsizei width,GLsizei height,GLenum format,GLsizei imageSize,const void* data){
	printf("Unimplemented: glCompressedTexSubImage2D\n");
}



SYS_PUBLIC void glCompressedTexSubImage3D(GLenum target,GLint level,GLint xoffset,GLint yoffset,GLint zoffset,GLsizei width,GLsizei height,GLsizei depth,GLenum format,GLsizei imageSize,const void* data){
	printf("Unimplemented: glCompressedTexSubImage3D\n");
}



SYS_PUBLIC void glCopyBufferSubData(GLenum readTarget,GLenum writeTarget,GLintptr readOffset,GLintptr writeOffset,GLsizeiptr size){
	printf("Unimplemented: glCopyBufferSubData\n");
}



SYS_PUBLIC void glCopyTexImage1D(GLenum target,GLint level,GLenum internalformat,GLint x,GLint y,GLsizei width,GLint border){
	printf("Unimplemented: glCopyTexImage1D\n");
}



SYS_PUBLIC void glCopyTexImage2D(GLenum target,GLint level,GLenum internalformat,GLint x,GLint y,GLsizei width,GLsizei height,GLint border){
	printf("Unimplemented: glCopyTexImage2D\n");
}



SYS_PUBLIC void glCopyTexSubImage1D(GLenum target,GLint level,GLint xoffset,GLint x,GLint y,GLsizei width){
	printf("Unimplemented: glCopyTexSubImage1D\n");
}



SYS_PUBLIC void glCopyTexSubImage2D(GLenum target,GLint level,GLint xoffset,GLint yoffset,GLint x,GLint y,GLsizei width,GLsizei height){
	printf("Unimplemented: glCopyTexSubImage2D\n");
}



SYS_PUBLIC void glCopyTexSubImage3D(GLenum target,GLint level,GLint xoffset,GLint yoffset,GLint zoffset,GLint x,GLint y,GLsizei width,GLsizei height){
	printf("Unimplemented: glCopyTexSubImage3D\n");
}



SYS_PUBLIC GLuint glCreateProgram(void){
	printf("Unimplemented: glCreateProgram\n");
	return 0;
}



SYS_PUBLIC GLuint glCreateShader(GLenum type){
	printf("Unimplemented: glCreateShader\n");
	return 0;
}



SYS_PUBLIC void glCullFace(GLenum mode){
	printf("Unimplemented: glCullFace\n");
}



SYS_PUBLIC void glDeleteBuffers(GLsizei n,const GLuint* buffers){
	printf("Unimplemented: glDeleteBuffers\n");
}



SYS_PUBLIC void glDeleteFramebuffers(GLsizei n,const GLuint* framebuffers){
	printf("Unimplemented: glDeleteFramebuffers\n");
}



SYS_PUBLIC void glDeleteProgram(GLuint program){
	printf("Unimplemented: glDeleteProgram\n");
}



SYS_PUBLIC void glDeleteQueries(GLsizei n,const GLuint* ids){
	printf("Unimplemented: glDeleteQueries\n");
}



SYS_PUBLIC void glDeleteRenderbuffers(GLsizei n,const GLuint* renderbuffers){
	printf("Unimplemented: glDeleteRenderbuffers\n");
}



SYS_PUBLIC void glDeleteSamplers(GLsizei count,const GLuint* samplers){
	printf("Unimplemented: glDeleteSamplers\n");
}



SYS_PUBLIC void glDeleteShader(GLuint shader){
	printf("Unimplemented: glDeleteShader\n");
}



SYS_PUBLIC void glDeleteSync(GLsync sync){
	printf("Unimplemented: glDeleteSync\n");
}



SYS_PUBLIC void glDeleteTextures(GLsizei n,const GLuint* textures){
	printf("Unimplemented: glDeleteTextures\n");
}



SYS_PUBLIC void glDeleteVertexArrays(GLsizei n,const GLuint* arrays){
	printf("Unimplemented: glDeleteVertexArrays\n");
}



SYS_PUBLIC void glDepthFunc(GLenum func){
	printf("Unimplemented: glDepthFunc\n");
}



SYS_PUBLIC void glDepthMask(GLboolean flag){
	printf("Unimplemented: glDepthMask\n");
}



SYS_PUBLIC void glDepthRange(GLdouble n,GLdouble f){
	printf("Unimplemented: glDepthRange\n");
}



SYS_PUBLIC void glDetachShader(GLuint program,GLuint shader){
	printf("Unimplemented: glDetachShader\n");
}



SYS_PUBLIC void glDisable(GLenum cap){
	printf("Unimplemented: glDisable\n");
}



SYS_PUBLIC void glDisablei(GLenum target,GLuint index){
	printf("Unimplemented: glDisablei\n");
}



SYS_PUBLIC void glDisableVertexAttribArray(GLuint index){
	printf("Unimplemented: glDisableVertexAttribArray\n");
}



SYS_PUBLIC void glDrawArrays(GLenum mode,GLint first,GLsizei count){
	printf("Unimplemented: glDrawArrays\n");
}



SYS_PUBLIC void glDrawArraysInstanced(GLenum mode,GLint first,GLsizei count,GLsizei instancecount){
	printf("Unimplemented: glDrawArraysInstanced\n");
}



SYS_PUBLIC void glDrawBuffer(GLenum buf){
	printf("Unimplemented: glDrawBuffer\n");
}



SYS_PUBLIC void glDrawBuffers(GLsizei n,const GLenum* bufs){
	printf("Unimplemented: glDrawBuffers\n");
}



SYS_PUBLIC void glDrawElements(GLenum mode,GLsizei count,GLenum type,const void* indices){
	printf("Unimplemented: glDrawElements\n");
}



SYS_PUBLIC void glDrawElementsBaseVertex(GLenum mode,GLsizei count,GLenum type,const void* indices,GLint basevertex){
	printf("Unimplemented: glDrawElementsBaseVertex\n");
}



SYS_PUBLIC void glDrawElementsInstanced(GLenum mode,GLsizei count,GLenum type,const void* indices,GLsizei instancecount){
	printf("Unimplemented: glDrawElementsInstanced\n");
}



SYS_PUBLIC void glDrawElementsInstancedBaseVertex(GLenum mode,GLsizei count,GLenum type,const void* indices,GLsizei instancecount,GLint basevertex){
	printf("Unimplemented: glDrawElementsInstancedBaseVertex\n");
}



SYS_PUBLIC void glDrawRangeElements(GLenum mode,GLuint start,GLuint end,GLsizei count,GLenum type,const void* indices){
	printf("Unimplemented: glDrawRangeElements\n");
}



SYS_PUBLIC void glDrawRangeElementsBaseVertex(GLenum mode,GLuint start,GLuint end,GLsizei count,GLenum type,const void* indices,GLint basevertex){
	printf("Unimplemented: glDrawRangeElementsBaseVertex\n");
}



SYS_PUBLIC void glEnable(GLenum cap){
	printf("Unimplemented: glEnable\n");
}



SYS_PUBLIC void glEnablei(GLenum target,GLuint index){
	printf("Unimplemented: glEnablei\n");
}



SYS_PUBLIC void glEnableVertexAttribArray(GLuint index){
	printf("Unimplemented: glEnableVertexAttribArray\n");
}



SYS_PUBLIC void glEndConditionalRender(void){
	printf("Unimplemented: glEndConditionalRender\n");
}



SYS_PUBLIC void glEndQuery(GLenum target){
	printf("Unimplemented: glEndQuery\n");
}



SYS_PUBLIC void glEndTransformFeedback(void){
	printf("Unimplemented: glEndTransformFeedback\n");
}



SYS_PUBLIC GLsync glFenceSync(GLenum condition,GLbitfield flags){
	printf("Unimplemented: glFenceSync\n");
	return 0;
}



SYS_PUBLIC void glFinish(void){
	printf("Unimplemented: glFinish\n");
}



SYS_PUBLIC void glFlush(void){
	printf("Unimplemented: glFlush\n");
}



SYS_PUBLIC void glFlushMappedBufferRange(GLenum target,GLintptr offset,GLsizeiptr length){
	printf("Unimplemented: glFlushMappedBufferRange\n");
}



SYS_PUBLIC void glFramebufferRenderbuffer(GLenum target,GLenum attachment,GLenum renderbuffertarget,GLuint renderbuffer){
	printf("Unimplemented: glFramebufferRenderbuffer\n");
}



SYS_PUBLIC void glFramebufferTexture(GLenum target,GLenum attachment,GLuint texture,GLint level){
	printf("Unimplemented: glFramebufferTexture\n");
}



SYS_PUBLIC void glFramebufferTexture1D(GLenum target,GLenum attachment,GLenum textarget,GLuint texture,GLint level){
	printf("Unimplemented: glFramebufferTexture1D\n");
}



SYS_PUBLIC void glFramebufferTexture2D(GLenum target,GLenum attachment,GLenum textarget,GLuint texture,GLint level){
	printf("Unimplemented: glFramebufferTexture2D\n");
}



SYS_PUBLIC void glFramebufferTexture3D(GLenum target,GLenum attachment,GLenum textarget,GLuint texture,GLint level,GLint zoffset){
	printf("Unimplemented: glFramebufferTexture3D\n");
}



SYS_PUBLIC void glFramebufferTextureLayer(GLenum target,GLenum attachment,GLuint texture,GLint level,GLint layer){
	printf("Unimplemented: glFramebufferTextureLayer\n");
}



SYS_PUBLIC void glFrontFace(GLenum mode){
	printf("Unimplemented: glFrontFace\n");
}



SYS_PUBLIC void glGenBuffers(GLsizei n,GLuint* buffers){
	printf("Unimplemented: glGenBuffers\n");
}



SYS_PUBLIC void glGenerateMipmap(GLenum target){
	printf("Unimplemented: glGenerateMipmap\n");
}



SYS_PUBLIC void glGenFramebuffers(GLsizei n,GLuint* framebuffers){
	printf("Unimplemented: glGenFramebuffers\n");
}



SYS_PUBLIC void glGenQueries(GLsizei n,GLuint* ids){
	printf("Unimplemented: glGenQueries\n");
}



SYS_PUBLIC void glGenRenderbuffers(GLsizei n,GLuint* renderbuffers){
	printf("Unimplemented: glGenRenderbuffers\n");
}



SYS_PUBLIC void glGenSamplers(GLsizei count,GLuint* samplers){
	printf("Unimplemented: glGenSamplers\n");
}



SYS_PUBLIC void glGenTextures(GLsizei n,GLuint* textures){
	printf("Unimplemented: glGenTextures\n");
}



SYS_PUBLIC void glGenVertexArrays(GLsizei n,GLuint* arrays){
	printf("Unimplemented: glGenVertexArrays\n");
}



SYS_PUBLIC void glGetActiveAttrib(GLuint program,GLuint index,GLsizei bufSize,GLsizei* length,GLint* size,GLenum* type,GLchar* name){
	printf("Unimplemented: glGetActiveAttrib\n");
}



SYS_PUBLIC void glGetActiveUniform(GLuint program,GLuint index,GLsizei bufSize,GLsizei* length,GLint* size,GLenum* type,GLchar* name){
	printf("Unimplemented: glGetActiveUniform\n");
}



SYS_PUBLIC void glGetActiveUniformBlockiv(GLuint program,GLuint uniformBlockIndex,GLenum pname,GLint* params){
	printf("Unimplemented: glGetActiveUniformBlockiv\n");
}



SYS_PUBLIC void glGetActiveUniformBlockName(GLuint program,GLuint uniformBlockIndex,GLsizei bufSize,GLsizei* length,GLchar* uniformBlockName){
	printf("Unimplemented: glGetActiveUniformBlockName\n");
}



SYS_PUBLIC void glGetActiveUniformName(GLuint program,GLuint uniformIndex,GLsizei bufSize,GLsizei* length,GLchar* uniformName){
	printf("Unimplemented: glGetActiveUniformName\n");
}



SYS_PUBLIC void glGetActiveUniformsiv(GLuint program,GLsizei uniformCount,const GLuint* uniformIndices,GLenum pname,GLint* params){
	printf("Unimplemented: glGetActiveUniformsiv\n");
}



SYS_PUBLIC void glGetAttachedShaders(GLuint program,GLsizei maxCount,GLsizei* count,GLuint* shaders){
	printf("Unimplemented: glGetAttachedShaders\n");
}



SYS_PUBLIC GLint glGetAttribLocation(GLuint program,const GLchar* name){
	printf("Unimplemented: glGetAttribLocation\n");
	return 0;
}



SYS_PUBLIC void glGetBooleani_v(GLenum target,GLuint index,GLboolean* data){
	printf("Unimplemented: glGetBooleani_v\n");
}



SYS_PUBLIC void glGetBooleanv(GLenum pname,GLboolean* data){
	printf("Unimplemented: glGetBooleanv\n");
}



SYS_PUBLIC void glGetBufferParameteri64v(GLenum target,GLenum pname,GLint64* params){
	printf("Unimplemented: glGetBufferParameteri64v\n");
}



SYS_PUBLIC void glGetBufferParameteriv(GLenum target,GLenum pname,GLint* params){
	printf("Unimplemented: glGetBufferParameteriv\n");
}



SYS_PUBLIC void glGetBufferPointerv(GLenum target,GLenum pname,void* *params){
	printf("Unimplemented: glGetBufferPointerv\n");
}



SYS_PUBLIC void glGetBufferSubData(GLenum target,GLintptr offset,GLsizeiptr size,void* data){
	printf("Unimplemented: glGetBufferSubData\n");
}



SYS_PUBLIC void glGetCompressedTexImage(GLenum target,GLint level,void* img){
	printf("Unimplemented: glGetCompressedTexImage\n");
}



SYS_PUBLIC void glGetDoublev(GLenum pname,GLdouble* data){
	printf("Unimplemented: glGetDoublev\n");
}



SYS_PUBLIC GLenum glGetError(void){
	printf("Unimplemented: glGetError\n");
	return 0;
}



SYS_PUBLIC void glGetFloatv(GLenum pname,GLfloat* data){
	printf("Unimplemented: glGetFloatv\n");
}



SYS_PUBLIC GLint glGetFragDataIndex(GLuint program,const GLchar* name){
	printf("Unimplemented: glGetFragDataIndex\n");
	return 0;
}



SYS_PUBLIC GLint glGetFragDataLocation(GLuint program,const GLchar* name){
	printf("Unimplemented: glGetFragDataLocation\n");
	return 0;
}



SYS_PUBLIC void glGetFramebufferAttachmentParameteriv(GLenum target,GLenum attachment,GLenum pname,GLint* params){
	printf("Unimplemented: glGetFramebufferAttachmentParameteriv\n");
}



SYS_PUBLIC void glGetInteger64i_v(GLenum target,GLuint index,GLint64* data){
	printf("Unimplemented: glGetInteger64i_v\n");
}



SYS_PUBLIC void glGetInteger64v(GLenum pname,GLint64* data){
	printf("Unimplemented: glGetInteger64v\n");
}



SYS_PUBLIC void glGetIntegeri_v(GLenum target,GLuint index,GLint* data){
	printf("Unimplemented: glGetIntegeri_v\n");
}



SYS_PUBLIC void glGetIntegerv(GLenum pname,GLint* data){
	printf("Unimplemented: glGetIntegerv\n");
}



SYS_PUBLIC void glGetMultisamplefv(GLenum pname,GLuint index,GLfloat* val){
	printf("Unimplemented: glGetMultisamplefv\n");
}



SYS_PUBLIC void glGetPointerv(GLenum pname,void* *params){
	printf("Unimplemented: glGetPointerv\n");
}



SYS_PUBLIC void glGetProgramInfoLog(GLuint program,GLsizei bufSize,GLsizei* length,GLchar* infoLog){
	printf("Unimplemented: glGetProgramInfoLog\n");
}



SYS_PUBLIC void glGetProgramiv(GLuint program,GLenum pname,GLint* params){
	printf("Unimplemented: glGetProgramiv\n");
}



SYS_PUBLIC void glGetQueryiv(GLenum target,GLenum pname,GLint* params){
	printf("Unimplemented: glGetQueryiv\n");
}



SYS_PUBLIC void glGetQueryObjecti64v(GLuint id,GLenum pname,GLint64* params){
	printf("Unimplemented: glGetQueryObjecti64v\n");
}



SYS_PUBLIC void glGetQueryObjectiv(GLuint id,GLenum pname,GLint* params){
	printf("Unimplemented: glGetQueryObjectiv\n");
}



SYS_PUBLIC void glGetQueryObjectui64v(GLuint id,GLenum pname,GLuint64* params){
	printf("Unimplemented: glGetQueryObjectui64v\n");
}



SYS_PUBLIC void glGetQueryObjectuiv(GLuint id,GLenum pname,GLuint* params){
	printf("Unimplemented: glGetQueryObjectuiv\n");
}



SYS_PUBLIC void glGetRenderbufferParameteriv(GLenum target,GLenum pname,GLint* params){
	printf("Unimplemented: glGetRenderbufferParameteriv\n");
}



SYS_PUBLIC void glGetSamplerParameterfv(GLuint sampler,GLenum pname,GLfloat* params){
	printf("Unimplemented: glGetSamplerParameterfv\n");
}



SYS_PUBLIC void glGetSamplerParameterIiv(GLuint sampler,GLenum pname,GLint* params){
	printf("Unimplemented: glGetSamplerParameterIiv\n");
}



SYS_PUBLIC void glGetSamplerParameterIuiv(GLuint sampler,GLenum pname,GLuint* params){
	printf("Unimplemented: glGetSamplerParameterIuiv\n");
}



SYS_PUBLIC void glGetSamplerParameteriv(GLuint sampler,GLenum pname,GLint* params){
	printf("Unimplemented: glGetSamplerParameteriv\n");
}



SYS_PUBLIC void glGetShaderInfoLog(GLuint shader,GLsizei bufSize,GLsizei* length,GLchar* infoLog){
	printf("Unimplemented: glGetShaderInfoLog\n");
}



SYS_PUBLIC void glGetShaderiv(GLuint shader,GLenum pname,GLint* params){
	printf("Unimplemented: glGetShaderiv\n");
}



SYS_PUBLIC void glGetShaderSource(GLuint shader,GLsizei bufSize,GLsizei* length,GLchar* source){
	printf("Unimplemented: glGetShaderSource\n");
}



SYS_PUBLIC const GLubyte* glGetString(GLenum name){
	printf("Unimplemented: glGetString\n");
	return NULL;
}



SYS_PUBLIC const GLubyte* glGetStringi(GLenum name,GLuint index){
	printf("Unimplemented: glGetStringi\n");
	return NULL;
}



SYS_PUBLIC void glGetSynciv(GLsync sync,GLenum pname,GLsizei count,GLsizei* length,GLint* values){
	printf("Unimplemented: glGetSynciv\n");
}



SYS_PUBLIC void glGetTexImage(GLenum target,GLint level,GLenum format,GLenum type,void* pixels){
	printf("Unimplemented: glGetTexImage\n");
}



SYS_PUBLIC void glGetTexLevelParameterfv(GLenum target,GLint level,GLenum pname,GLfloat* params){
	printf("Unimplemented: glGetTexLevelParameterfv\n");
}



SYS_PUBLIC void glGetTexLevelParameteriv(GLenum target,GLint level,GLenum pname,GLint* params){
	printf("Unimplemented: glGetTexLevelParameteriv\n");
}



SYS_PUBLIC void glGetTexParameterfv(GLenum target,GLenum pname,GLfloat* params){
	printf("Unimplemented: glGetTexParameterfv\n");
}



SYS_PUBLIC void glGetTexParameterIiv(GLenum target,GLenum pname,GLint* params){
	printf("Unimplemented: glGetTexParameterIiv\n");
}



SYS_PUBLIC void glGetTexParameterIuiv(GLenum target,GLenum pname,GLuint* params){
	printf("Unimplemented: glGetTexParameterIuiv\n");
}



SYS_PUBLIC void glGetTexParameteriv(GLenum target,GLenum pname,GLint* params){
	printf("Unimplemented: glGetTexParameteriv\n");
}



SYS_PUBLIC void glGetTransformFeedbackVarying(GLuint program,GLuint index,GLsizei bufSize,GLsizei* length,GLsizei* size,GLenum* type,GLchar* name){
	printf("Unimplemented: glGetTransformFeedbackVarying\n");
}



SYS_PUBLIC GLuint glGetUniformBlockIndex(GLuint program,const GLchar* uniformBlockName){
	printf("Unimplemented: glGetUniformBlockIndex\n");
	return 0;
}



SYS_PUBLIC void glGetUniformfv(GLuint program,GLint location,GLfloat* params){
	printf("Unimplemented: glGetUniformfv\n");
}



SYS_PUBLIC void glGetUniformIndices(GLuint program,GLsizei uniformCount,const GLchar* const*uniformNames,GLuint* uniformIndices){
	printf("Unimplemented: glGetUniformIndices\n");
}



SYS_PUBLIC void glGetUniformiv(GLuint program,GLint location,GLint* params){
	printf("Unimplemented: glGetUniformiv\n");
}



SYS_PUBLIC GLint glGetUniformLocation(GLuint program,const GLchar* name){
	printf("Unimplemented: glGetUniformLocation\n");
	return 0;
}



SYS_PUBLIC void glGetUniformuiv(GLuint program,GLint location,GLuint* params){
	printf("Unimplemented: glGetUniformuiv\n");
}



SYS_PUBLIC void glGetVertexAttribdv(GLuint index,GLenum pname,GLdouble* params){
	printf("Unimplemented: glGetVertexAttribdv\n");
}



SYS_PUBLIC void glGetVertexAttribfv(GLuint index,GLenum pname,GLfloat* params){
	printf("Unimplemented: glGetVertexAttribfv\n");
}



SYS_PUBLIC void glGetVertexAttribIiv(GLuint index,GLenum pname,GLint* params){
	printf("Unimplemented: glGetVertexAttribIiv\n");
}



SYS_PUBLIC void glGetVertexAttribIuiv(GLuint index,GLenum pname,GLuint* params){
	printf("Unimplemented: glGetVertexAttribIuiv\n");
}



SYS_PUBLIC void glGetVertexAttribiv(GLuint index,GLenum pname,GLint* params){
	printf("Unimplemented: glGetVertexAttribiv\n");
}



SYS_PUBLIC void glGetVertexAttribPointerv(GLuint index,GLenum pname,void* *pointer){
	printf("Unimplemented: glGetVertexAttribPointerv\n");
}



SYS_PUBLIC void glHint(GLenum target,GLenum mode){
	printf("Unimplemented: glHint\n");
}



SYS_PUBLIC GLboolean glIsBuffer(GLuint buffer){
	printf("Unimplemented: glIsBuffer\n");
	return 0;
}



SYS_PUBLIC GLboolean glIsEnabled(GLenum cap){
	printf("Unimplemented: glIsEnabled\n");
	return 0;
}



SYS_PUBLIC GLboolean glIsEnabledi(GLenum target,GLuint index){
	printf("Unimplemented: glIsEnabledi\n");
	return 0;
}



SYS_PUBLIC GLboolean glIsFramebuffer(GLuint framebuffer){
	printf("Unimplemented: glIsFramebuffer\n");
	return 0;
}



SYS_PUBLIC GLboolean glIsProgram(GLuint program){
	printf("Unimplemented: glIsProgram\n");
	return 0;
}



SYS_PUBLIC GLboolean glIsQuery(GLuint id){
	printf("Unimplemented: glIsQuery\n");
	return 0;
}



SYS_PUBLIC GLboolean glIsRenderbuffer(GLuint renderbuffer){
	printf("Unimplemented: glIsRenderbuffer\n");
	return 0;
}



SYS_PUBLIC GLboolean glIsSampler(GLuint sampler){
	printf("Unimplemented: glIsSampler\n");
	return 0;
}



SYS_PUBLIC GLboolean glIsShader(GLuint shader){
	printf("Unimplemented: glIsShader\n");
	return 0;
}



SYS_PUBLIC GLboolean glIsSync(GLsync sync){
	printf("Unimplemented: glIsSync\n");
	return 0;
}



SYS_PUBLIC GLboolean glIsTexture(GLuint texture){
	printf("Unimplemented: glIsTexture\n");
	return 0;
}



SYS_PUBLIC GLboolean glIsVertexArray(GLuint array){
	printf("Unimplemented: glIsVertexArray\n");
	return 0;
}



SYS_PUBLIC void glLineWidth(GLfloat width){
	printf("Unimplemented: glLineWidth\n");
}



SYS_PUBLIC void glLinkProgram(GLuint program){
	printf("Unimplemented: glLinkProgram\n");
}



SYS_PUBLIC void glLogicOp(GLenum opcode){
	printf("Unimplemented: glLogicOp\n");
}



SYS_PUBLIC void* glMapBuffer(GLenum target,GLenum access){
	printf("Unimplemented: glMapBuffer\n");
	return NULL;
}



SYS_PUBLIC void* glMapBufferRange(GLenum target,GLintptr offset,GLsizeiptr length,GLbitfield access){
	printf("Unimplemented: glMapBufferRange\n");
	return NULL;
}



SYS_PUBLIC void glMultiDrawArrays(GLenum mode,const GLint* first,const GLsizei* count,GLsizei drawcount){
	printf("Unimplemented: glMultiDrawArrays\n");
}



SYS_PUBLIC void glMultiDrawElements(GLenum mode,const GLsizei* count,GLenum type,const void* const*indices,GLsizei drawcount){
	printf("Unimplemented: glMultiDrawElements\n");
}



SYS_PUBLIC void glMultiDrawElementsBaseVertex(GLenum mode,const GLsizei* count,GLenum type,const void* const*indices,GLsizei drawcount,const GLint* basevertex){
	printf("Unimplemented: glMultiDrawElementsBaseVertex\n");
}



SYS_PUBLIC void glPixelStoref(GLenum pname,GLfloat param){
	printf("Unimplemented: glPixelStoref\n");
}



SYS_PUBLIC void glPixelStorei(GLenum pname,GLint param){
	printf("Unimplemented: glPixelStorei\n");
}



SYS_PUBLIC void glPointParameterf(GLenum pname,GLfloat param){
	printf("Unimplemented: glPointParameterf\n");
}



SYS_PUBLIC void glPointParameterfv(GLenum pname,const GLfloat* params){
	printf("Unimplemented: glPointParameterfv\n");
}



SYS_PUBLIC void glPointParameteri(GLenum pname,GLint param){
	printf("Unimplemented: glPointParameteri\n");
}



SYS_PUBLIC void glPointParameteriv(GLenum pname,const GLint* params){
	printf("Unimplemented: glPointParameteriv\n");
}



SYS_PUBLIC void glPointSize(GLfloat size){
	printf("Unimplemented: glPointSize\n");
}



SYS_PUBLIC void glPolygonMode(GLenum face,GLenum mode){
	printf("Unimplemented: glPolygonMode\n");
}



SYS_PUBLIC void glPolygonOffset(GLfloat factor,GLfloat units){
	printf("Unimplemented: glPolygonOffset\n");
}



SYS_PUBLIC void glPrimitiveRestartIndex(GLuint index){
	printf("Unimplemented: glPrimitiveRestartIndex\n");
}



SYS_PUBLIC void glProvokingVertex(GLenum mode){
	printf("Unimplemented: glProvokingVertex\n");
}



SYS_PUBLIC void glQueryCounter(GLuint id,GLenum target){
	printf("Unimplemented: glQueryCounter\n");
}



SYS_PUBLIC void glReadBuffer(GLenum src){
	printf("Unimplemented: glReadBuffer\n");
}



SYS_PUBLIC void glReadPixels(GLint x,GLint y,GLsizei width,GLsizei height,GLenum format,GLenum type,void* pixels){
	printf("Unimplemented: glReadPixels\n");
}



SYS_PUBLIC void glRenderbufferStorage(GLenum target,GLenum internalformat,GLsizei width,GLsizei height){
	printf("Unimplemented: glRenderbufferStorage\n");
}



SYS_PUBLIC void glRenderbufferStorageMultisample(GLenum target,GLsizei samples,GLenum internalformat,GLsizei width,GLsizei height){
	printf("Unimplemented: glRenderbufferStorageMultisample\n");
}



SYS_PUBLIC void glSampleCoverage(GLfloat value,GLboolean invert){
	printf("Unimplemented: glSampleCoverage\n");
}



SYS_PUBLIC void glSampleMaski(GLuint maskNumber,GLbitfield mask){
	printf("Unimplemented: glSampleMaski\n");
}



SYS_PUBLIC void glSamplerParameterf(GLuint sampler,GLenum pname,GLfloat param){
	printf("Unimplemented: glSamplerParameterf\n");
}



SYS_PUBLIC void glSamplerParameterfv(GLuint sampler,GLenum pname,const GLfloat* param){
	printf("Unimplemented: glSamplerParameterfv\n");
}



SYS_PUBLIC void glSamplerParameteri(GLuint sampler,GLenum pname,GLint param){
	printf("Unimplemented: glSamplerParameteri\n");
}



SYS_PUBLIC void glSamplerParameterIiv(GLuint sampler,GLenum pname,const GLint* param){
	printf("Unimplemented: glSamplerParameterIiv\n");
}



SYS_PUBLIC void glSamplerParameterIuiv(GLuint sampler,GLenum pname,const GLuint* param){
	printf("Unimplemented: glSamplerParameterIuiv\n");
}



SYS_PUBLIC void glSamplerParameteriv(GLuint sampler,GLenum pname,const GLint* param){
	printf("Unimplemented: glSamplerParameteriv\n");
}



SYS_PUBLIC void glScissor(GLint x,GLint y,GLsizei width,GLsizei height){
	printf("Unimplemented: glScissor\n");
}



SYS_PUBLIC void glShaderSource(GLuint shader,GLsizei count,const GLchar* const*string,const GLint* length){
	printf("Unimplemented: glShaderSource\n");
}



SYS_PUBLIC void glStencilFunc(GLenum func,GLint ref,GLuint mask){
	printf("Unimplemented: glStencilFunc\n");
}



SYS_PUBLIC void glStencilFuncSeparate(GLenum face,GLenum func,GLint ref,GLuint mask){
	printf("Unimplemented: glStencilFuncSeparate\n");
}



SYS_PUBLIC void glStencilMask(GLuint mask){
	printf("Unimplemented: glStencilMask\n");
}



SYS_PUBLIC void glStencilMaskSeparate(GLenum face,GLuint mask){
	printf("Unimplemented: glStencilMaskSeparate\n");
}



SYS_PUBLIC void glStencilOp(GLenum fail,GLenum zfail,GLenum zpass){
	printf("Unimplemented: glStencilOp\n");
}



SYS_PUBLIC void glStencilOpSeparate(GLenum face,GLenum sfail,GLenum dpfail,GLenum dppass){
	printf("Unimplemented: glStencilOpSeparate\n");
}



SYS_PUBLIC void glTexBuffer(GLenum target,GLenum internalformat,GLuint buffer){
	printf("Unimplemented: glTexBuffer\n");
}



SYS_PUBLIC void glTexImage1D(GLenum target,GLint level,GLint internalformat,GLsizei width,GLint border,GLenum format,GLenum type,const void* pixels){
	printf("Unimplemented: glTexImage1D\n");
}



SYS_PUBLIC void glTexImage2D(GLenum target,GLint level,GLint internalformat,GLsizei width,GLsizei height,GLint border,GLenum format,GLenum type,const void* pixels){
	printf("Unimplemented: glTexImage2D\n");
}



SYS_PUBLIC void glTexImage2DMultisample(GLenum target,GLsizei samples,GLenum internalformat,GLsizei width,GLsizei height,GLboolean fixedsamplelocations){
	printf("Unimplemented: glTexImage2DMultisample\n");
}



SYS_PUBLIC void glTexImage3D(GLenum target,GLint level,GLint internalformat,GLsizei width,GLsizei height,GLsizei depth,GLint border,GLenum format,GLenum type,const void* pixels){
	printf("Unimplemented: glTexImage3D\n");
}



SYS_PUBLIC void glTexImage3DMultisample(GLenum target,GLsizei samples,GLenum internalformat,GLsizei width,GLsizei height,GLsizei depth,GLboolean fixedsamplelocations){
	printf("Unimplemented: glTexImage3DMultisample\n");
}



SYS_PUBLIC void glTexParameterf(GLenum target,GLenum pname,GLfloat param){
	printf("Unimplemented: glTexParameterf\n");
}



SYS_PUBLIC void glTexParameterfv(GLenum target,GLenum pname,const GLfloat* params){
	printf("Unimplemented: glTexParameterfv\n");
}



SYS_PUBLIC void glTexParameteri(GLenum target,GLenum pname,GLint param){
	printf("Unimplemented: glTexParameteri\n");
}



SYS_PUBLIC void glTexParameterIiv(GLenum target,GLenum pname,const GLint* params){
	printf("Unimplemented: glTexParameterIiv\n");
}



SYS_PUBLIC void glTexParameterIuiv(GLenum target,GLenum pname,const GLuint* params){
	printf("Unimplemented: glTexParameterIuiv\n");
}



SYS_PUBLIC void glTexParameteriv(GLenum target,GLenum pname,const GLint* params){
	printf("Unimplemented: glTexParameteriv\n");
}



SYS_PUBLIC void glTexSubImage1D(GLenum target,GLint level,GLint xoffset,GLsizei width,GLenum format,GLenum type,const void* pixels){
	printf("Unimplemented: glTexSubImage1D\n");
}



SYS_PUBLIC void glTexSubImage2D(GLenum target,GLint level,GLint xoffset,GLint yoffset,GLsizei width,GLsizei height,GLenum format,GLenum type,const void* pixels){
	printf("Unimplemented: glTexSubImage2D\n");
}



SYS_PUBLIC void glTexSubImage3D(GLenum target,GLint level,GLint xoffset,GLint yoffset,GLint zoffset,GLsizei width,GLsizei height,GLsizei depth,GLenum format,GLenum type,const void* pixels){
	printf("Unimplemented: glTexSubImage3D\n");
}



SYS_PUBLIC void glTransformFeedbackVaryings(GLuint program,GLsizei count,const GLchar* const*varyings,GLenum bufferMode){
	printf("Unimplemented: glTransformFeedbackVaryings\n");
}



SYS_PUBLIC void glUniform1f(GLint location,GLfloat v0){
	printf("Unimplemented: glUniform1f\n");
}



SYS_PUBLIC void glUniform1fv(GLint location,GLsizei count,const GLfloat* value){
	printf("Unimplemented: glUniform1fv\n");
}



SYS_PUBLIC void glUniform1i(GLint location,GLint v0){
	printf("Unimplemented: glUniform1i\n");
}



SYS_PUBLIC void glUniform1iv(GLint location,GLsizei count,const GLint* value){
	printf("Unimplemented: glUniform1iv\n");
}



SYS_PUBLIC void glUniform1ui(GLint location,GLuint v0){
	printf("Unimplemented: glUniform1ui\n");
}



SYS_PUBLIC void glUniform1uiv(GLint location,GLsizei count,const GLuint* value){
	printf("Unimplemented: glUniform1uiv\n");
}



SYS_PUBLIC void glUniform2f(GLint location,GLfloat v0,GLfloat v1){
	printf("Unimplemented: glUniform2f\n");
}



SYS_PUBLIC void glUniform2fv(GLint location,GLsizei count,const GLfloat* value){
	printf("Unimplemented: glUniform2fv\n");
}



SYS_PUBLIC void glUniform2i(GLint location,GLint v0,GLint v1){
	printf("Unimplemented: glUniform2i\n");
}



SYS_PUBLIC void glUniform2iv(GLint location,GLsizei count,const GLint* value){
	printf("Unimplemented: glUniform2iv\n");
}



SYS_PUBLIC void glUniform2ui(GLint location,GLuint v0,GLuint v1){
	printf("Unimplemented: glUniform2ui\n");
}



SYS_PUBLIC void glUniform2uiv(GLint location,GLsizei count,const GLuint* value){
	printf("Unimplemented: glUniform2uiv\n");
}



SYS_PUBLIC void glUniform3f(GLint location,GLfloat v0,GLfloat v1,GLfloat v2){
	printf("Unimplemented: glUniform3f\n");
}



SYS_PUBLIC void glUniform3fv(GLint location,GLsizei count,const GLfloat* value){
	printf("Unimplemented: glUniform3fv\n");
}



SYS_PUBLIC void glUniform3i(GLint location,GLint v0,GLint v1,GLint v2){
	printf("Unimplemented: glUniform3i\n");
}



SYS_PUBLIC void glUniform3iv(GLint location,GLsizei count,const GLint* value){
	printf("Unimplemented: glUniform3iv\n");
}



SYS_PUBLIC void glUniform3ui(GLint location,GLuint v0,GLuint v1,GLuint v2){
	printf("Unimplemented: glUniform3ui\n");
}



SYS_PUBLIC void glUniform3uiv(GLint location,GLsizei count,const GLuint* value){
	printf("Unimplemented: glUniform3uiv\n");
}



SYS_PUBLIC void glUniform4f(GLint location,GLfloat v0,GLfloat v1,GLfloat v2,GLfloat v3){
	printf("Unimplemented: glUniform4f\n");
}



SYS_PUBLIC void glUniform4fv(GLint location,GLsizei count,const GLfloat* value){
	printf("Unimplemented: glUniform4fv\n");
}



SYS_PUBLIC void glUniform4i(GLint location,GLint v0,GLint v1,GLint v2,GLint v3){
	printf("Unimplemented: glUniform4i\n");
}



SYS_PUBLIC void glUniform4iv(GLint location,GLsizei count,const GLint* value){
	printf("Unimplemented: glUniform4iv\n");
}



SYS_PUBLIC void glUniform4ui(GLint location,GLuint v0,GLuint v1,GLuint v2,GLuint v3){
	printf("Unimplemented: glUniform4ui\n");
}



SYS_PUBLIC void glUniform4uiv(GLint location,GLsizei count,const GLuint* value){
	printf("Unimplemented: glUniform4uiv\n");
}



SYS_PUBLIC void glUniformBlockBinding(GLuint program,GLuint uniformBlockIndex,GLuint uniformBlockBinding){
	printf("Unimplemented: glUniformBlockBinding\n");
}



SYS_PUBLIC void glUniformMatrix2fv(GLint location,GLsizei count,GLboolean transpose,const GLfloat* value){
	printf("Unimplemented: glUniformMatrix2fv\n");
}



SYS_PUBLIC void glUniformMatrix2x3fv(GLint location,GLsizei count,GLboolean transpose,const GLfloat* value){
	printf("Unimplemented: glUniformMatrix2x3fv\n");
}



SYS_PUBLIC void glUniformMatrix2x4fv(GLint location,GLsizei count,GLboolean transpose,const GLfloat* value){
	printf("Unimplemented: glUniformMatrix2x4fv\n");
}



SYS_PUBLIC void glUniformMatrix3fv(GLint location,GLsizei count,GLboolean transpose,const GLfloat* value){
	printf("Unimplemented: glUniformMatrix3fv\n");
}



SYS_PUBLIC void glUniformMatrix3x2fv(GLint location,GLsizei count,GLboolean transpose,const GLfloat* value){
	printf("Unimplemented: glUniformMatrix3x2fv\n");
}



SYS_PUBLIC void glUniformMatrix3x4fv(GLint location,GLsizei count,GLboolean transpose,const GLfloat* value){
	printf("Unimplemented: glUniformMatrix3x4fv\n");
}



SYS_PUBLIC void glUniformMatrix4fv(GLint location,GLsizei count,GLboolean transpose,const GLfloat* value){
	printf("Unimplemented: glUniformMatrix4fv\n");
}



SYS_PUBLIC void glUniformMatrix4x2fv(GLint location,GLsizei count,GLboolean transpose,const GLfloat* value){
	printf("Unimplemented: glUniformMatrix4x2fv\n");
}



SYS_PUBLIC void glUniformMatrix4x3fv(GLint location,GLsizei count,GLboolean transpose,const GLfloat* value){
	printf("Unimplemented: glUniformMatrix4x3fv\n");
}



SYS_PUBLIC GLboolean glUnmapBuffer(GLenum target){
	printf("Unimplemented: glUnmapBuffer\n");
	return 0;
}



SYS_PUBLIC void glUseProgram(GLuint program){
	printf("Unimplemented: glUseProgram\n");
}



SYS_PUBLIC void glValidateProgram(GLuint program){
	printf("Unimplemented: glValidateProgram\n");
}



SYS_PUBLIC void glVertexAttrib1d(GLuint index,GLdouble x){
	printf("Unimplemented: glVertexAttrib1d\n");
}



SYS_PUBLIC void glVertexAttrib1dv(GLuint index,const GLdouble* v){
	printf("Unimplemented: glVertexAttrib1dv\n");
}



SYS_PUBLIC void glVertexAttrib1f(GLuint index,GLfloat x){
	printf("Unimplemented: glVertexAttrib1f\n");
}



SYS_PUBLIC void glVertexAttrib1fv(GLuint index,const GLfloat* v){
	printf("Unimplemented: glVertexAttrib1fv\n");
}



SYS_PUBLIC void glVertexAttrib1s(GLuint index,GLshort x){
	printf("Unimplemented: glVertexAttrib1s\n");
}



SYS_PUBLIC void glVertexAttrib1sv(GLuint index,const GLshort* v){
	printf("Unimplemented: glVertexAttrib1sv\n");
}



SYS_PUBLIC void glVertexAttrib2d(GLuint index,GLdouble x,GLdouble y){
	printf("Unimplemented: glVertexAttrib2d\n");
}



SYS_PUBLIC void glVertexAttrib2dv(GLuint index,const GLdouble* v){
	printf("Unimplemented: glVertexAttrib2dv\n");
}



SYS_PUBLIC void glVertexAttrib2f(GLuint index,GLfloat x,GLfloat y){
	printf("Unimplemented: glVertexAttrib2f\n");
}



SYS_PUBLIC void glVertexAttrib2fv(GLuint index,const GLfloat* v){
	printf("Unimplemented: glVertexAttrib2fv\n");
}



SYS_PUBLIC void glVertexAttrib2s(GLuint index,GLshort x,GLshort y){
	printf("Unimplemented: glVertexAttrib2s\n");
}



SYS_PUBLIC void glVertexAttrib2sv(GLuint index,const GLshort* v){
	printf("Unimplemented: glVertexAttrib2sv\n");
}



SYS_PUBLIC void glVertexAttrib3d(GLuint index,GLdouble x,GLdouble y,GLdouble z){
	printf("Unimplemented: glVertexAttrib3d\n");
}



SYS_PUBLIC void glVertexAttrib3dv(GLuint index,const GLdouble* v){
	printf("Unimplemented: glVertexAttrib3dv\n");
}



SYS_PUBLIC void glVertexAttrib3f(GLuint index,GLfloat x,GLfloat y,GLfloat z){
	printf("Unimplemented: glVertexAttrib3f\n");
}



SYS_PUBLIC void glVertexAttrib3fv(GLuint index,const GLfloat* v){
	printf("Unimplemented: glVertexAttrib3fv\n");
}



SYS_PUBLIC void glVertexAttrib3s(GLuint index,GLshort x,GLshort y,GLshort z){
	printf("Unimplemented: glVertexAttrib3s\n");
}



SYS_PUBLIC void glVertexAttrib3sv(GLuint index,const GLshort* v){
	printf("Unimplemented: glVertexAttrib3sv\n");
}



SYS_PUBLIC void glVertexAttrib4bv(GLuint index,const GLbyte* v){
	printf("Unimplemented: glVertexAttrib4bv\n");
}



SYS_PUBLIC void glVertexAttrib4d(GLuint index,GLdouble x,GLdouble y,GLdouble z,GLdouble w){
	printf("Unimplemented: glVertexAttrib4d\n");
}



SYS_PUBLIC void glVertexAttrib4dv(GLuint index,const GLdouble* v){
	printf("Unimplemented: glVertexAttrib4dv\n");
}



SYS_PUBLIC void glVertexAttrib4f(GLuint index,GLfloat x,GLfloat y,GLfloat z,GLfloat w){
	printf("Unimplemented: glVertexAttrib4f\n");
}



SYS_PUBLIC void glVertexAttrib4fv(GLuint index,const GLfloat* v){
	printf("Unimplemented: glVertexAttrib4fv\n");
}



SYS_PUBLIC void glVertexAttrib4iv(GLuint index,const GLint* v){
	printf("Unimplemented: glVertexAttrib4iv\n");
}



SYS_PUBLIC void glVertexAttrib4Nbv(GLuint index,const GLbyte* v){
	printf("Unimplemented: glVertexAttrib4Nbv\n");
}



SYS_PUBLIC void glVertexAttrib4Niv(GLuint index,const GLint* v){
	printf("Unimplemented: glVertexAttrib4Niv\n");
}



SYS_PUBLIC void glVertexAttrib4Nsv(GLuint index,const GLshort* v){
	printf("Unimplemented: glVertexAttrib4Nsv\n");
}



SYS_PUBLIC void glVertexAttrib4Nub(GLuint index,GLubyte x,GLubyte y,GLubyte z,GLubyte w){
	printf("Unimplemented: glVertexAttrib4Nub\n");
}



SYS_PUBLIC void glVertexAttrib4Nubv(GLuint index,const GLubyte* v){
	printf("Unimplemented: glVertexAttrib4Nubv\n");
}



SYS_PUBLIC void glVertexAttrib4Nuiv(GLuint index,const GLuint* v){
	printf("Unimplemented: glVertexAttrib4Nuiv\n");
}



SYS_PUBLIC void glVertexAttrib4Nusv(GLuint index,const GLushort* v){
	printf("Unimplemented: glVertexAttrib4Nusv\n");
}



SYS_PUBLIC void glVertexAttrib4s(GLuint index,GLshort x,GLshort y,GLshort z,GLshort w){
	printf("Unimplemented: glVertexAttrib4s\n");
}



SYS_PUBLIC void glVertexAttrib4sv(GLuint index,const GLshort* v){
	printf("Unimplemented: glVertexAttrib4sv\n");
}



SYS_PUBLIC void glVertexAttrib4ubv(GLuint index,const GLubyte* v){
	printf("Unimplemented: glVertexAttrib4ubv\n");
}



SYS_PUBLIC void glVertexAttrib4uiv(GLuint index,const GLuint* v){
	printf("Unimplemented: glVertexAttrib4uiv\n");
}



SYS_PUBLIC void glVertexAttrib4usv(GLuint index,const GLushort* v){
	printf("Unimplemented: glVertexAttrib4usv\n");
}



SYS_PUBLIC void glVertexAttribDivisor(GLuint index,GLuint divisor){
	printf("Unimplemented: glVertexAttribDivisor\n");
}



SYS_PUBLIC void glVertexAttribI1i(GLuint index,GLint x){
	printf("Unimplemented: glVertexAttribI1i\n");
}



SYS_PUBLIC void glVertexAttribI1iv(GLuint index,const GLint* v){
	printf("Unimplemented: glVertexAttribI1iv\n");
}



SYS_PUBLIC void glVertexAttribI1ui(GLuint index,GLuint x){
	printf("Unimplemented: glVertexAttribI1ui\n");
}



SYS_PUBLIC void glVertexAttribI1uiv(GLuint index,const GLuint* v){
	printf("Unimplemented: glVertexAttribI1uiv\n");
}



SYS_PUBLIC void glVertexAttribI2i(GLuint index,GLint x,GLint y){
	printf("Unimplemented: glVertexAttribI2i\n");
}



SYS_PUBLIC void glVertexAttribI2iv(GLuint index,const GLint* v){
	printf("Unimplemented: glVertexAttribI2iv\n");
}



SYS_PUBLIC void glVertexAttribI2ui(GLuint index,GLuint x,GLuint y){
	printf("Unimplemented: glVertexAttribI2ui\n");
}



SYS_PUBLIC void glVertexAttribI2uiv(GLuint index,const GLuint* v){
	printf("Unimplemented: glVertexAttribI2uiv\n");
}



SYS_PUBLIC void glVertexAttribI3i(GLuint index,GLint x,GLint y,GLint z){
	printf("Unimplemented: glVertexAttribI3i\n");
}



SYS_PUBLIC void glVertexAttribI3iv(GLuint index,const GLint* v){
	printf("Unimplemented: glVertexAttribI3iv\n");
}



SYS_PUBLIC void glVertexAttribI3ui(GLuint index,GLuint x,GLuint y,GLuint z){
	printf("Unimplemented: glVertexAttribI3ui\n");
}



SYS_PUBLIC void glVertexAttribI3uiv(GLuint index,const GLuint* v){
	printf("Unimplemented: glVertexAttribI3uiv\n");
}



SYS_PUBLIC void glVertexAttribI4bv(GLuint index,const GLbyte* v){
	printf("Unimplemented: glVertexAttribI4bv\n");
}



SYS_PUBLIC void glVertexAttribI4i(GLuint index,GLint x,GLint y,GLint z,GLint w){
	printf("Unimplemented: glVertexAttribI4i\n");
}



SYS_PUBLIC void glVertexAttribI4iv(GLuint index,const GLint* v){
	printf("Unimplemented: glVertexAttribI4iv\n");
}



SYS_PUBLIC void glVertexAttribI4sv(GLuint index,const GLshort* v){
	printf("Unimplemented: glVertexAttribI4sv\n");
}



SYS_PUBLIC void glVertexAttribI4ubv(GLuint index,const GLubyte* v){
	printf("Unimplemented: glVertexAttribI4ubv\n");
}



SYS_PUBLIC void glVertexAttribI4ui(GLuint index,GLuint x,GLuint y,GLuint z,GLuint w){
	printf("Unimplemented: glVertexAttribI4ui\n");
}



SYS_PUBLIC void glVertexAttribI4uiv(GLuint index,const GLuint* v){
	printf("Unimplemented: glVertexAttribI4uiv\n");
}



SYS_PUBLIC void glVertexAttribI4usv(GLuint index,const GLushort* v){
	printf("Unimplemented: glVertexAttribI4usv\n");
}



SYS_PUBLIC void glVertexAttribIPointer(GLuint index,GLint size,GLenum type,GLsizei stride,const void* pointer){
	printf("Unimplemented: glVertexAttribIPointer\n");
}



SYS_PUBLIC void glVertexAttribP1ui(GLuint index,GLenum type,GLboolean normalized,GLuint value){
	printf("Unimplemented: glVertexAttribP1ui\n");
}



SYS_PUBLIC void glVertexAttribP1uiv(GLuint index,GLenum type,GLboolean normalized,const GLuint* value){
	printf("Unimplemented: glVertexAttribP1uiv\n");
}



SYS_PUBLIC void glVertexAttribP2ui(GLuint index,GLenum type,GLboolean normalized,GLuint value){
	printf("Unimplemented: glVertexAttribP2ui\n");
}



SYS_PUBLIC void glVertexAttribP2uiv(GLuint index,GLenum type,GLboolean normalized,const GLuint* value){
	printf("Unimplemented: glVertexAttribP2uiv\n");
}



SYS_PUBLIC void glVertexAttribP3ui(GLuint index,GLenum type,GLboolean normalized,GLuint value){
	printf("Unimplemented: glVertexAttribP3ui\n");
}



SYS_PUBLIC void glVertexAttribP3uiv(GLuint index,GLenum type,GLboolean normalized,const GLuint* value){
	printf("Unimplemented: glVertexAttribP3uiv\n");
}



SYS_PUBLIC void glVertexAttribP4ui(GLuint index,GLenum type,GLboolean normalized,GLuint value){
	printf("Unimplemented: glVertexAttribP4ui\n");
}



SYS_PUBLIC void glVertexAttribP4uiv(GLuint index,GLenum type,GLboolean normalized,const GLuint* value){
	printf("Unimplemented: glVertexAttribP4uiv\n");
}



SYS_PUBLIC void glVertexAttribPointer(GLuint index,GLint size,GLenum type,GLboolean normalized,GLsizei stride,const void* pointer){
	printf("Unimplemented: glVertexAttribPointer\n");
}



SYS_PUBLIC void glViewport(GLint x,GLint y,GLsizei width,GLsizei height){
	printf("Unimplemented: glViewport\n");
}



SYS_PUBLIC void glWaitSync(GLsync sync,GLbitfield flags,GLuint64 timeout){
	printf("Unimplemented: glWaitSync\n");
}
