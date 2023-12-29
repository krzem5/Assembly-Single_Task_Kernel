#ifndef _GL_GL_H_
#define _GL_GL_H_ 1
#include <GL/glcorearb.h>



const GLubyte* APIENTRY glGetString(GLenum name);



const GLubyte* APIENTRY glGetStringi(GLenum name,GLuint index);



GLboolean APIENTRY glIsBuffer(GLuint buffer);



GLboolean APIENTRY glIsEnabled(GLenum cap);



GLboolean APIENTRY glIsEnabledi(GLenum target,GLuint index);



GLboolean APIENTRY glIsFramebuffer(GLuint framebuffer);



GLboolean APIENTRY glIsProgram(GLuint program);



GLboolean APIENTRY glIsQuery(GLuint id);



GLboolean APIENTRY glIsRenderbuffer(GLuint renderbuffer);



GLboolean APIENTRY glIsSampler(GLuint sampler);



GLboolean APIENTRY glIsShader(GLuint shader);



GLboolean APIENTRY glIsSync(GLsync sync);



GLboolean APIENTRY glIsTexture(GLuint texture);



GLboolean APIENTRY glIsVertexArray(GLuint array);



GLboolean APIENTRY glUnmapBuffer(GLenum target);



GLenum APIENTRY glCheckFramebufferStatus(GLenum target);



GLenum APIENTRY glClientWaitSync(GLsync sync,GLbitfield flags,GLuint64 timeout);



GLenum APIENTRY glGetError(void);



GLint APIENTRY glGetAttribLocation(GLuint program,const GLchar* name);



GLint APIENTRY glGetFragDataIndex(GLuint program,const GLchar* name);



GLint APIENTRY glGetFragDataLocation(GLuint program,const GLchar* name);



GLint APIENTRY glGetUniformLocation(GLuint program,const GLchar* name);



GLsync APIENTRY glFenceSync(GLenum condition,GLbitfield flags);



GLuint APIENTRY glCreateProgram(void);



GLuint APIENTRY glCreateShader(GLenum type);



GLuint APIENTRY glGetUniformBlockIndex(GLuint program,const GLchar* uniformBlockName);



void* APIENTRY glMapBuffer(GLenum target,GLenum access);



void* APIENTRY glMapBufferRange(GLenum target,GLintptr offset,GLsizeiptr length,GLbitfield access);



void APIENTRY glActiveTexture(GLenum texture);



void APIENTRY glAttachShader(GLuint program,GLuint shader);



void APIENTRY glBeginConditionalRender(GLuint id,GLenum mode);



void APIENTRY glBeginQuery(GLenum target,GLuint id);



void APIENTRY glBeginTransformFeedback(GLenum primitiveMode);



void APIENTRY glBindAttribLocation(GLuint program,GLuint index,const GLchar* name);



void APIENTRY glBindBuffer(GLenum target,GLuint buffer);



void APIENTRY glBindBufferBase(GLenum target,GLuint index,GLuint buffer);



void APIENTRY glBindBufferRange(GLenum target,GLuint index,GLuint buffer,GLintptr offset,GLsizeiptr size);



void APIENTRY glBindFragDataLocation(GLuint program,GLuint color,const GLchar* name);



void APIENTRY glBindFragDataLocationIndexed(GLuint program,GLuint colorNumber,GLuint index,const GLchar* name);



void APIENTRY glBindFramebuffer(GLenum target,GLuint framebuffer);



void APIENTRY glBindRenderbuffer(GLenum target,GLuint renderbuffer);



void APIENTRY glBindSampler(GLuint unit,GLuint sampler);



void APIENTRY glBindTexture(GLenum target,GLuint texture);



void APIENTRY glBindVertexArray(GLuint array);



void APIENTRY glBlendColor(GLfloat red,GLfloat green,GLfloat blue,GLfloat alpha);



void APIENTRY glBlendEquation(GLenum mode);



void APIENTRY glBlendEquationSeparate(GLenum modeRGB,GLenum modeAlpha);



void APIENTRY glBlendFunc(GLenum sfactor,GLenum dfactor);



void APIENTRY glBlendFuncSeparate(GLenum sfactorRGB,GLenum dfactorRGB,GLenum sfactorAlpha,GLenum dfactorAlpha);



void APIENTRY glBlitFramebuffer(GLint srcX0,GLint srcY0,GLint srcX1,GLint srcY1,GLint dstX0,GLint dstY0,GLint dstX1,GLint dstY1,GLbitfield mask,GLenum filter);



void APIENTRY glBufferData(GLenum target,GLsizeiptr size,const void* data,GLenum usage);



void APIENTRY glBufferSubData(GLenum target,GLintptr offset,GLsizeiptr size,const void* data);



void APIENTRY glClampColor(GLenum target,GLenum clamp);



void APIENTRY glClear(GLbitfield mask);



void APIENTRY glClearBufferfi(GLenum buffer,GLint drawbuffer,GLfloat depth,GLint stencil);



void APIENTRY glClearBufferfv(GLenum buffer,GLint drawbuffer,const GLfloat* value);



void APIENTRY glClearBufferiv(GLenum buffer,GLint drawbuffer,const GLint* value);



void APIENTRY glClearBufferuiv(GLenum buffer,GLint drawbuffer,const GLuint* value);



void APIENTRY glClearColor(GLfloat red,GLfloat green,GLfloat blue,GLfloat alpha);



void APIENTRY glClearDepth(GLdouble depth);



void APIENTRY glClearStencil(GLint s);



void APIENTRY glColorMask(GLboolean red,GLboolean green,GLboolean blue,GLboolean alpha);



void APIENTRY glColorMaski(GLuint index,GLboolean r,GLboolean g,GLboolean b,GLboolean a);



void APIENTRY glCompileShader(GLuint shader);



void APIENTRY glCompressedTexImage1D(GLenum target,GLint level,GLenum internalformat,GLsizei width,GLint border,GLsizei imageSize,const void* data);



void APIENTRY glCompressedTexImage2D(GLenum target,GLint level,GLenum internalformat,GLsizei width,GLsizei height,GLint border,GLsizei imageSize,const void* data);



void APIENTRY glCompressedTexImage3D(GLenum target,GLint level,GLenum internalformat,GLsizei width,GLsizei height,GLsizei depth,GLint border,GLsizei imageSize,const void* data);



void APIENTRY glCompressedTexSubImage1D(GLenum target,GLint level,GLint xoffset,GLsizei width,GLenum format,GLsizei imageSize,const void* data);



void APIENTRY glCompressedTexSubImage2D(GLenum target,GLint level,GLint xoffset,GLint yoffset,GLsizei width,GLsizei height,GLenum format,GLsizei imageSize,const void* data);



void APIENTRY glCompressedTexSubImage3D(GLenum target,GLint level,GLint xoffset,GLint yoffset,GLint zoffset,GLsizei width,GLsizei height,GLsizei depth,GLenum format,GLsizei imageSize,const void* data);



void APIENTRY glCopyBufferSubData(GLenum readTarget,GLenum writeTarget,GLintptr readOffset,GLintptr writeOffset,GLsizeiptr size);



void APIENTRY glCopyTexImage1D(GLenum target,GLint level,GLenum internalformat,GLint x,GLint y,GLsizei width,GLint border);



void APIENTRY glCopyTexImage2D(GLenum target,GLint level,GLenum internalformat,GLint x,GLint y,GLsizei width,GLsizei height,GLint border);



void APIENTRY glCopyTexSubImage1D(GLenum target,GLint level,GLint xoffset,GLint x,GLint y,GLsizei width);



void APIENTRY glCopyTexSubImage2D(GLenum target,GLint level,GLint xoffset,GLint yoffset,GLint x,GLint y,GLsizei width,GLsizei height);



void APIENTRY glCopyTexSubImage3D(GLenum target,GLint level,GLint xoffset,GLint yoffset,GLint zoffset,GLint x,GLint y,GLsizei width,GLsizei height);



void APIENTRY glCullFace(GLenum mode);



void APIENTRY glDeleteBuffers(GLsizei n,const GLuint* buffers);



void APIENTRY glDeleteFramebuffers(GLsizei n,const GLuint* framebuffers);



void APIENTRY glDeleteProgram(GLuint program);



void APIENTRY glDeleteQueries(GLsizei n,const GLuint* ids);



void APIENTRY glDeleteRenderbuffers(GLsizei n,const GLuint* renderbuffers);



void APIENTRY glDeleteSamplers(GLsizei count,const GLuint* samplers);



void APIENTRY glDeleteShader(GLuint shader);



void APIENTRY glDeleteSync(GLsync sync);



void APIENTRY glDeleteTextures(GLsizei n,const GLuint* textures);



void APIENTRY glDeleteVertexArrays(GLsizei n,const GLuint* arrays);



void APIENTRY glDepthFunc(GLenum func);



void APIENTRY glDepthMask(GLboolean flag);



void APIENTRY glDepthRange(GLdouble n,GLdouble f);



void APIENTRY glDetachShader(GLuint program,GLuint shader);



void APIENTRY glDisable(GLenum cap);



void APIENTRY glDisablei(GLenum target,GLuint index);



void APIENTRY glDisableVertexAttribArray(GLuint index);



void APIENTRY glDrawArrays(GLenum mode,GLint first,GLsizei count);



void APIENTRY glDrawArraysInstanced(GLenum mode,GLint first,GLsizei count,GLsizei instancecount);



void APIENTRY glDrawBuffer(GLenum buf);



void APIENTRY glDrawBuffers(GLsizei n,const GLenum* bufs);



void APIENTRY glDrawElements(GLenum mode,GLsizei count,GLenum type,const void* indices);



void APIENTRY glDrawElementsBaseVertex(GLenum mode,GLsizei count,GLenum type,const void* indices,GLint basevertex);



void APIENTRY glDrawElementsInstanced(GLenum mode,GLsizei count,GLenum type,const void* indices,GLsizei instancecount);



void APIENTRY glDrawElementsInstancedBaseVertex(GLenum mode,GLsizei count,GLenum type,const void* indices,GLsizei instancecount,GLint basevertex);



void APIENTRY glDrawRangeElements(GLenum mode,GLuint start,GLuint end,GLsizei count,GLenum type,const void* indices);



void APIENTRY glDrawRangeElementsBaseVertex(GLenum mode,GLuint start,GLuint end,GLsizei count,GLenum type,const void* indices,GLint basevertex);



void APIENTRY glEnable(GLenum cap);



void APIENTRY glEnablei(GLenum target,GLuint index);



void APIENTRY glEnableVertexAttribArray(GLuint index);



void APIENTRY glEndConditionalRender(void);



void APIENTRY glEndQuery(GLenum target);



void APIENTRY glEndTransformFeedback(void);



void APIENTRY glFinish(void);



void APIENTRY glFlush(void);



void APIENTRY glFlushMappedBufferRange(GLenum target,GLintptr offset,GLsizeiptr length);



void APIENTRY glFramebufferRenderbuffer(GLenum target,GLenum attachment,GLenum renderbuffertarget,GLuint renderbuffer);



void APIENTRY glFramebufferTexture(GLenum target,GLenum attachment,GLuint texture,GLint level);



void APIENTRY glFramebufferTexture1D(GLenum target,GLenum attachment,GLenum textarget,GLuint texture,GLint level);



void APIENTRY glFramebufferTexture2D(GLenum target,GLenum attachment,GLenum textarget,GLuint texture,GLint level);



void APIENTRY glFramebufferTexture3D(GLenum target,GLenum attachment,GLenum textarget,GLuint texture,GLint level,GLint zoffset);



void APIENTRY glFramebufferTextureLayer(GLenum target,GLenum attachment,GLuint texture,GLint level,GLint layer);



void APIENTRY glFrontFace(GLenum mode);



void APIENTRY glGenBuffers(GLsizei n,GLuint* buffers);



void APIENTRY glGenerateMipmap(GLenum target);



void APIENTRY glGenFramebuffers(GLsizei n,GLuint* framebuffers);



void APIENTRY glGenQueries(GLsizei n,GLuint* ids);



void APIENTRY glGenRenderbuffers(GLsizei n,GLuint* renderbuffers);



void APIENTRY glGenSamplers(GLsizei count,GLuint* samplers);



void APIENTRY glGenTextures(GLsizei n,GLuint* textures);



void APIENTRY glGenVertexArrays(GLsizei n,GLuint* arrays);



void APIENTRY glGetActiveAttrib(GLuint program,GLuint index,GLsizei bufSize,GLsizei* length,GLint* size,GLenum* type,GLchar* name);



void APIENTRY glGetActiveUniform(GLuint program,GLuint index,GLsizei bufSize,GLsizei* length,GLint* size,GLenum* type,GLchar* name);



void APIENTRY glGetActiveUniformBlockiv(GLuint program,GLuint uniformBlockIndex,GLenum pname,GLint* params);



void APIENTRY glGetActiveUniformBlockName(GLuint program,GLuint uniformBlockIndex,GLsizei bufSize,GLsizei* length,GLchar* uniformBlockName);



void APIENTRY glGetActiveUniformName(GLuint program,GLuint uniformIndex,GLsizei bufSize,GLsizei* length,GLchar* uniformName);



void APIENTRY glGetActiveUniformsiv(GLuint program,GLsizei uniformCount,const GLuint* uniformIndices,GLenum pname,GLint* params);



void APIENTRY glGetAttachedShaders(GLuint program,GLsizei maxCount,GLsizei* count,GLuint* shaders);



void APIENTRY glGetBooleani_v(GLenum target,GLuint index,GLboolean* data);



void APIENTRY glGetBooleanv(GLenum pname,GLboolean* data);



void APIENTRY glGetBufferParameteri64v(GLenum target,GLenum pname,GLint64* params);



void APIENTRY glGetBufferParameteriv(GLenum target,GLenum pname,GLint* params);



void APIENTRY glGetBufferPointerv(GLenum target,GLenum pname,void* *params);



void APIENTRY glGetBufferSubData(GLenum target,GLintptr offset,GLsizeiptr size,void* data);



void APIENTRY glGetCompressedTexImage(GLenum target,GLint level,void* img);



void APIENTRY glGetDoublev(GLenum pname,GLdouble* data);



void APIENTRY glGetFloatv(GLenum pname,GLfloat* data);



void APIENTRY glGetFramebufferAttachmentParameteriv(GLenum target,GLenum attachment,GLenum pname,GLint* params);



void APIENTRY glGetInteger64i_v(GLenum target,GLuint index,GLint64* data);



void APIENTRY glGetInteger64v(GLenum pname,GLint64* data);



void APIENTRY glGetIntegeri_v(GLenum target,GLuint index,GLint* data);



void APIENTRY glGetIntegerv(GLenum pname,GLint* data);



void APIENTRY glGetMultisamplefv(GLenum pname,GLuint index,GLfloat* val);



void APIENTRY glGetPointerv(GLenum pname,void* *params);



void APIENTRY glGetProgramInfoLog(GLuint program,GLsizei bufSize,GLsizei* length,GLchar* infoLog);



void APIENTRY glGetProgramiv(GLuint program,GLenum pname,GLint* params);



void APIENTRY glGetQueryiv(GLenum target,GLenum pname,GLint* params);



void APIENTRY glGetQueryObjecti64v(GLuint id,GLenum pname,GLint64* params);



void APIENTRY glGetQueryObjectiv(GLuint id,GLenum pname,GLint* params);



void APIENTRY glGetQueryObjectui64v(GLuint id,GLenum pname,GLuint64* params);



void APIENTRY glGetQueryObjectuiv(GLuint id,GLenum pname,GLuint* params);



void APIENTRY glGetRenderbufferParameteriv(GLenum target,GLenum pname,GLint* params);



void APIENTRY glGetSamplerParameterfv(GLuint sampler,GLenum pname,GLfloat* params);



void APIENTRY glGetSamplerParameterIiv(GLuint sampler,GLenum pname,GLint* params);



void APIENTRY glGetSamplerParameterIuiv(GLuint sampler,GLenum pname,GLuint* params);



void APIENTRY glGetSamplerParameteriv(GLuint sampler,GLenum pname,GLint* params);



void APIENTRY glGetShaderInfoLog(GLuint shader,GLsizei bufSize,GLsizei* length,GLchar* infoLog);



void APIENTRY glGetShaderiv(GLuint shader,GLenum pname,GLint* params);



void APIENTRY glGetShaderSource(GLuint shader,GLsizei bufSize,GLsizei* length,GLchar* source);



void APIENTRY glGetSynciv(GLsync sync,GLenum pname,GLsizei count,GLsizei* length,GLint* values);



void APIENTRY glGetTexImage(GLenum target,GLint level,GLenum format,GLenum type,void* pixels);



void APIENTRY glGetTexLevelParameterfv(GLenum target,GLint level,GLenum pname,GLfloat* params);



void APIENTRY glGetTexLevelParameteriv(GLenum target,GLint level,GLenum pname,GLint* params);



void APIENTRY glGetTexParameterfv(GLenum target,GLenum pname,GLfloat* params);



void APIENTRY glGetTexParameterIiv(GLenum target,GLenum pname,GLint* params);



void APIENTRY glGetTexParameterIuiv(GLenum target,GLenum pname,GLuint* params);



void APIENTRY glGetTexParameteriv(GLenum target,GLenum pname,GLint* params);



void APIENTRY glGetTransformFeedbackVarying(GLuint program,GLuint index,GLsizei bufSize,GLsizei* length,GLsizei* size,GLenum* type,GLchar* name);



void APIENTRY glGetUniformfv(GLuint program,GLint location,GLfloat* params);



void APIENTRY glGetUniformIndices(GLuint program,GLsizei uniformCount,const GLchar* const*uniformNames,GLuint* uniformIndices);



void APIENTRY glGetUniformiv(GLuint program,GLint location,GLint* params);



void APIENTRY glGetUniformuiv(GLuint program,GLint location,GLuint* params);



void APIENTRY glGetVertexAttribdv(GLuint index,GLenum pname,GLdouble* params);



void APIENTRY glGetVertexAttribfv(GLuint index,GLenum pname,GLfloat* params);



void APIENTRY glGetVertexAttribIiv(GLuint index,GLenum pname,GLint* params);



void APIENTRY glGetVertexAttribIuiv(GLuint index,GLenum pname,GLuint* params);



void APIENTRY glGetVertexAttribiv(GLuint index,GLenum pname,GLint* params);



void APIENTRY glGetVertexAttribPointerv(GLuint index,GLenum pname,void* *pointer);



void APIENTRY glHint(GLenum target,GLenum mode);



void APIENTRY glLineWidth(GLfloat width);



void APIENTRY glLinkProgram(GLuint program);



void APIENTRY glLogicOp(GLenum opcode);



void APIENTRY glMultiDrawArrays(GLenum mode,const GLint* first,const GLsizei* count,GLsizei drawcount);



void APIENTRY glMultiDrawElements(GLenum mode,const GLsizei* count,GLenum type,const void* const*indices,GLsizei drawcount);



void APIENTRY glMultiDrawElementsBaseVertex(GLenum mode,const GLsizei* count,GLenum type,const void* const*indices,GLsizei drawcount,const GLint* basevertex);



void APIENTRY glPixelStoref(GLenum pname,GLfloat param);



void APIENTRY glPixelStorei(GLenum pname,GLint param);



void APIENTRY glPointParameterf(GLenum pname,GLfloat param);



void APIENTRY glPointParameterfv(GLenum pname,const GLfloat* params);



void APIENTRY glPointParameteri(GLenum pname,GLint param);



void APIENTRY glPointParameteriv(GLenum pname,const GLint* params);



void APIENTRY glPointSize(GLfloat size);



void APIENTRY glPolygonMode(GLenum face,GLenum mode);



void APIENTRY glPolygonOffset(GLfloat factor,GLfloat units);



void APIENTRY glPrimitiveRestartIndex(GLuint index);



void APIENTRY glProvokingVertex(GLenum mode);



void APIENTRY glQueryCounter(GLuint id,GLenum target);



void APIENTRY glReadBuffer(GLenum src);



void APIENTRY glReadPixels(GLint x,GLint y,GLsizei width,GLsizei height,GLenum format,GLenum type,void* pixels);



void APIENTRY glRenderbufferStorage(GLenum target,GLenum internalformat,GLsizei width,GLsizei height);



void APIENTRY glRenderbufferStorageMultisample(GLenum target,GLsizei samples,GLenum internalformat,GLsizei width,GLsizei height);



void APIENTRY glSampleCoverage(GLfloat value,GLboolean invert);



void APIENTRY glSampleMaski(GLuint maskNumber,GLbitfield mask);



void APIENTRY glSamplerParameterf(GLuint sampler,GLenum pname,GLfloat param);



void APIENTRY glSamplerParameterfv(GLuint sampler,GLenum pname,const GLfloat* param);



void APIENTRY glSamplerParameteri(GLuint sampler,GLenum pname,GLint param);



void APIENTRY glSamplerParameterIiv(GLuint sampler,GLenum pname,const GLint* param);



void APIENTRY glSamplerParameterIuiv(GLuint sampler,GLenum pname,const GLuint* param);



void APIENTRY glSamplerParameteriv(GLuint sampler,GLenum pname,const GLint* param);



void APIENTRY glScissor(GLint x,GLint y,GLsizei width,GLsizei height);



void APIENTRY glShaderSource(GLuint shader,GLsizei count,const GLchar* const*string,const GLint* length);



void APIENTRY glStencilFunc(GLenum func,GLint ref,GLuint mask);



void APIENTRY glStencilFuncSeparate(GLenum face,GLenum func,GLint ref,GLuint mask);



void APIENTRY glStencilMask(GLuint mask);



void APIENTRY glStencilMaskSeparate(GLenum face,GLuint mask);



void APIENTRY glStencilOp(GLenum fail,GLenum zfail,GLenum zpass);



void APIENTRY glStencilOpSeparate(GLenum face,GLenum sfail,GLenum dpfail,GLenum dppass);



void APIENTRY glTexBuffer(GLenum target,GLenum internalformat,GLuint buffer);



void APIENTRY glTexImage1D(GLenum target,GLint level,GLint internalformat,GLsizei width,GLint border,GLenum format,GLenum type,const void* pixels);



void APIENTRY glTexImage2D(GLenum target,GLint level,GLint internalformat,GLsizei width,GLsizei height,GLint border,GLenum format,GLenum type,const void* pixels);



void APIENTRY glTexImage2DMultisample(GLenum target,GLsizei samples,GLenum internalformat,GLsizei width,GLsizei height,GLboolean fixedsamplelocations);



void APIENTRY glTexImage3D(GLenum target,GLint level,GLint internalformat,GLsizei width,GLsizei height,GLsizei depth,GLint border,GLenum format,GLenum type,const void* pixels);



void APIENTRY glTexImage3DMultisample(GLenum target,GLsizei samples,GLenum internalformat,GLsizei width,GLsizei height,GLsizei depth,GLboolean fixedsamplelocations);



void APIENTRY glTexParameterf(GLenum target,GLenum pname,GLfloat param);



void APIENTRY glTexParameterfv(GLenum target,GLenum pname,const GLfloat* params);



void APIENTRY glTexParameteri(GLenum target,GLenum pname,GLint param);



void APIENTRY glTexParameterIiv(GLenum target,GLenum pname,const GLint* params);



void APIENTRY glTexParameterIuiv(GLenum target,GLenum pname,const GLuint* params);



void APIENTRY glTexParameteriv(GLenum target,GLenum pname,const GLint* params);



void APIENTRY glTexSubImage1D(GLenum target,GLint level,GLint xoffset,GLsizei width,GLenum format,GLenum type,const void* pixels);



void APIENTRY glTexSubImage2D(GLenum target,GLint level,GLint xoffset,GLint yoffset,GLsizei width,GLsizei height,GLenum format,GLenum type,const void* pixels);



void APIENTRY glTexSubImage3D(GLenum target,GLint level,GLint xoffset,GLint yoffset,GLint zoffset,GLsizei width,GLsizei height,GLsizei depth,GLenum format,GLenum type,const void* pixels);



void APIENTRY glTransformFeedbackVaryings(GLuint program,GLsizei count,const GLchar* const*varyings,GLenum bufferMode);



void APIENTRY glUniform1f(GLint location,GLfloat v0);



void APIENTRY glUniform1fv(GLint location,GLsizei count,const GLfloat* value);



void APIENTRY glUniform1i(GLint location,GLint v0);



void APIENTRY glUniform1iv(GLint location,GLsizei count,const GLint* value);



void APIENTRY glUniform1ui(GLint location,GLuint v0);



void APIENTRY glUniform1uiv(GLint location,GLsizei count,const GLuint* value);



void APIENTRY glUniform2f(GLint location,GLfloat v0,GLfloat v1);



void APIENTRY glUniform2fv(GLint location,GLsizei count,const GLfloat* value);



void APIENTRY glUniform2i(GLint location,GLint v0,GLint v1);



void APIENTRY glUniform2iv(GLint location,GLsizei count,const GLint* value);



void APIENTRY glUniform2ui(GLint location,GLuint v0,GLuint v1);



void APIENTRY glUniform2uiv(GLint location,GLsizei count,const GLuint* value);



void APIENTRY glUniform3f(GLint location,GLfloat v0,GLfloat v1,GLfloat v2);



void APIENTRY glUniform3fv(GLint location,GLsizei count,const GLfloat* value);



void APIENTRY glUniform3i(GLint location,GLint v0,GLint v1,GLint v2);



void APIENTRY glUniform3iv(GLint location,GLsizei count,const GLint* value);



void APIENTRY glUniform3ui(GLint location,GLuint v0,GLuint v1,GLuint v2);



void APIENTRY glUniform3uiv(GLint location,GLsizei count,const GLuint* value);



void APIENTRY glUniform4f(GLint location,GLfloat v0,GLfloat v1,GLfloat v2,GLfloat v3);



void APIENTRY glUniform4fv(GLint location,GLsizei count,const GLfloat* value);



void APIENTRY glUniform4i(GLint location,GLint v0,GLint v1,GLint v2,GLint v3);



void APIENTRY glUniform4iv(GLint location,GLsizei count,const GLint* value);



void APIENTRY glUniform4ui(GLint location,GLuint v0,GLuint v1,GLuint v2,GLuint v3);



void APIENTRY glUniform4uiv(GLint location,GLsizei count,const GLuint* value);



void APIENTRY glUniformBlockBinding(GLuint program,GLuint uniformBlockIndex,GLuint uniformBlockBinding);



void APIENTRY glUniformMatrix2fv(GLint location,GLsizei count,GLboolean transpose,const GLfloat* value);



void APIENTRY glUniformMatrix2x3fv(GLint location,GLsizei count,GLboolean transpose,const GLfloat* value);



void APIENTRY glUniformMatrix2x4fv(GLint location,GLsizei count,GLboolean transpose,const GLfloat* value);



void APIENTRY glUniformMatrix3fv(GLint location,GLsizei count,GLboolean transpose,const GLfloat* value);



void APIENTRY glUniformMatrix3x2fv(GLint location,GLsizei count,GLboolean transpose,const GLfloat* value);



void APIENTRY glUniformMatrix3x4fv(GLint location,GLsizei count,GLboolean transpose,const GLfloat* value);



void APIENTRY glUniformMatrix4fv(GLint location,GLsizei count,GLboolean transpose,const GLfloat* value);



void APIENTRY glUniformMatrix4x2fv(GLint location,GLsizei count,GLboolean transpose,const GLfloat* value);



void APIENTRY glUniformMatrix4x3fv(GLint location,GLsizei count,GLboolean transpose,const GLfloat* value);



void APIENTRY glUseProgram(GLuint program);



void APIENTRY glValidateProgram(GLuint program);



void APIENTRY glVertexAttrib1d(GLuint index,GLdouble x);



void APIENTRY glVertexAttrib1dv(GLuint index,const GLdouble* v);



void APIENTRY glVertexAttrib1f(GLuint index,GLfloat x);



void APIENTRY glVertexAttrib1fv(GLuint index,const GLfloat* v);



void APIENTRY glVertexAttrib1s(GLuint index,GLshort x);



void APIENTRY glVertexAttrib1sv(GLuint index,const GLshort* v);



void APIENTRY glVertexAttrib2d(GLuint index,GLdouble x,GLdouble y);



void APIENTRY glVertexAttrib2dv(GLuint index,const GLdouble* v);



void APIENTRY glVertexAttrib2f(GLuint index,GLfloat x,GLfloat y);



void APIENTRY glVertexAttrib2fv(GLuint index,const GLfloat* v);



void APIENTRY glVertexAttrib2s(GLuint index,GLshort x,GLshort y);



void APIENTRY glVertexAttrib2sv(GLuint index,const GLshort* v);



void APIENTRY glVertexAttrib3d(GLuint index,GLdouble x,GLdouble y,GLdouble z);



void APIENTRY glVertexAttrib3dv(GLuint index,const GLdouble* v);



void APIENTRY glVertexAttrib3f(GLuint index,GLfloat x,GLfloat y,GLfloat z);



void APIENTRY glVertexAttrib3fv(GLuint index,const GLfloat* v);



void APIENTRY glVertexAttrib3s(GLuint index,GLshort x,GLshort y,GLshort z);



void APIENTRY glVertexAttrib3sv(GLuint index,const GLshort* v);



void APIENTRY glVertexAttrib4bv(GLuint index,const GLbyte* v);



void APIENTRY glVertexAttrib4d(GLuint index,GLdouble x,GLdouble y,GLdouble z,GLdouble w);



void APIENTRY glVertexAttrib4dv(GLuint index,const GLdouble* v);



void APIENTRY glVertexAttrib4f(GLuint index,GLfloat x,GLfloat y,GLfloat z,GLfloat w);



void APIENTRY glVertexAttrib4fv(GLuint index,const GLfloat* v);



void APIENTRY glVertexAttrib4iv(GLuint index,const GLint* v);



void APIENTRY glVertexAttrib4Nbv(GLuint index,const GLbyte* v);



void APIENTRY glVertexAttrib4Niv(GLuint index,const GLint* v);



void APIENTRY glVertexAttrib4Nsv(GLuint index,const GLshort* v);



void APIENTRY glVertexAttrib4Nub(GLuint index,GLubyte x,GLubyte y,GLubyte z,GLubyte w);



void APIENTRY glVertexAttrib4Nubv(GLuint index,const GLubyte* v);



void APIENTRY glVertexAttrib4Nuiv(GLuint index,const GLuint* v);



void APIENTRY glVertexAttrib4Nusv(GLuint index,const GLushort* v);



void APIENTRY glVertexAttrib4s(GLuint index,GLshort x,GLshort y,GLshort z,GLshort w);



void APIENTRY glVertexAttrib4sv(GLuint index,const GLshort* v);



void APIENTRY glVertexAttrib4ubv(GLuint index,const GLubyte* v);



void APIENTRY glVertexAttrib4uiv(GLuint index,const GLuint* v);



void APIENTRY glVertexAttrib4usv(GLuint index,const GLushort* v);



void APIENTRY glVertexAttribDivisor(GLuint index,GLuint divisor);



void APIENTRY glVertexAttribI1i(GLuint index,GLint x);



void APIENTRY glVertexAttribI1iv(GLuint index,const GLint* v);



void APIENTRY glVertexAttribI1ui(GLuint index,GLuint x);



void APIENTRY glVertexAttribI1uiv(GLuint index,const GLuint* v);



void APIENTRY glVertexAttribI2i(GLuint index,GLint x,GLint y);



void APIENTRY glVertexAttribI2iv(GLuint index,const GLint* v);



void APIENTRY glVertexAttribI2ui(GLuint index,GLuint x,GLuint y);



void APIENTRY glVertexAttribI2uiv(GLuint index,const GLuint* v);



void APIENTRY glVertexAttribI3i(GLuint index,GLint x,GLint y,GLint z);



void APIENTRY glVertexAttribI3iv(GLuint index,const GLint* v);



void APIENTRY glVertexAttribI3ui(GLuint index,GLuint x,GLuint y,GLuint z);



void APIENTRY glVertexAttribI3uiv(GLuint index,const GLuint* v);



void APIENTRY glVertexAttribI4bv(GLuint index,const GLbyte* v);



void APIENTRY glVertexAttribI4i(GLuint index,GLint x,GLint y,GLint z,GLint w);



void APIENTRY glVertexAttribI4iv(GLuint index,const GLint* v);



void APIENTRY glVertexAttribI4sv(GLuint index,const GLshort* v);



void APIENTRY glVertexAttribI4ubv(GLuint index,const GLubyte* v);



void APIENTRY glVertexAttribI4ui(GLuint index,GLuint x,GLuint y,GLuint z,GLuint w);



void APIENTRY glVertexAttribI4uiv(GLuint index,const GLuint* v);



void APIENTRY glVertexAttribI4usv(GLuint index,const GLushort* v);



void APIENTRY glVertexAttribIPointer(GLuint index,GLint size,GLenum type,GLsizei stride,const void* pointer);



void APIENTRY glVertexAttribP1ui(GLuint index,GLenum type,GLboolean normalized,GLuint value);



void APIENTRY glVertexAttribP1uiv(GLuint index,GLenum type,GLboolean normalized,const GLuint* value);



void APIENTRY glVertexAttribP2ui(GLuint index,GLenum type,GLboolean normalized,GLuint value);



void APIENTRY glVertexAttribP2uiv(GLuint index,GLenum type,GLboolean normalized,const GLuint* value);



void APIENTRY glVertexAttribP3ui(GLuint index,GLenum type,GLboolean normalized,GLuint value);



void APIENTRY glVertexAttribP3uiv(GLuint index,GLenum type,GLboolean normalized,const GLuint* value);



void APIENTRY glVertexAttribP4ui(GLuint index,GLenum type,GLboolean normalized,GLuint value);



void APIENTRY glVertexAttribP4uiv(GLuint index,GLenum type,GLboolean normalized,const GLuint* value);



void APIENTRY glVertexAttribPointer(GLuint index,GLint size,GLenum type,GLboolean normalized,GLsizei stride,const void* pointer);



void APIENTRY glViewport(GLint x,GLint y,GLsizei width,GLsizei height);



void APIENTRY glWaitSync(GLsync sync,GLbitfield flags,GLuint64 timeout);



#endif
