#ifndef _GL_GL_H_
#define _GL_GL_H_ 1
#include <GL/glcorearb.h>



void glActiveTexture(GLenum texture);



void glAttachShader(GLuint program,GLuint shader);



void glBeginConditionalRender(GLuint id,GLenum mode);



void glBeginQuery(GLenum target,GLuint id);



void glBeginTransformFeedback(GLenum primitiveMode);



void glBindAttribLocation(GLuint program,GLuint index,const GLchar* name);



void glBindBuffer(GLenum target,GLuint buffer);



void glBindBufferBase(GLenum target,GLuint index,GLuint buffer);



void glBindBufferRange(GLenum target,GLuint index,GLuint buffer,GLintptr offset,GLsizeiptr size);



void glBindFragDataLocation(GLuint program,GLuint color,const GLchar* name);



void glBindFragDataLocationIndexed(GLuint program,GLuint colorNumber,GLuint index,const GLchar* name);



void glBindFramebuffer(GLenum target,GLuint framebuffer);



void glBindRenderbuffer(GLenum target,GLuint renderbuffer);



void glBindSampler(GLuint unit,GLuint sampler);



void glBindTexture(GLenum target,GLuint texture);



void glBindVertexArray(GLuint array);



void glBlendColor(GLfloat red,GLfloat green,GLfloat blue,GLfloat alpha);



void glBlendEquation(GLenum mode);



void glBlendEquationSeparate(GLenum modeRGB,GLenum modeAlpha);



void glBlendFunc(GLenum sfactor,GLenum dfactor);



void glBlendFuncSeparate(GLenum sfactorRGB,GLenum dfactorRGB,GLenum sfactorAlpha,GLenum dfactorAlpha);



void glBlitFramebuffer(GLint srcX0,GLint srcY0,GLint srcX1,GLint srcY1,GLint dstX0,GLint dstY0,GLint dstX1,GLint dstY1,GLbitfield mask,GLenum filter);



void glBufferData(GLenum target,GLsizeiptr size,const void* data,GLenum usage);



void glBufferSubData(GLenum target,GLintptr offset,GLsizeiptr size,const void* data);



GLenum glCheckFramebufferStatus(GLenum target);



void glClampColor(GLenum target,GLenum clamp);



void glClear(GLbitfield mask);



void glClearBufferfi(GLenum buffer,GLint drawbuffer,GLfloat depth,GLint stencil);



void glClearBufferfv(GLenum buffer,GLint drawbuffer,const GLfloat* value);



void glClearBufferiv(GLenum buffer,GLint drawbuffer,const GLint* value);



void glClearBufferuiv(GLenum buffer,GLint drawbuffer,const GLuint* value);



void glClearColor(GLfloat red,GLfloat green,GLfloat blue,GLfloat alpha);



void glClearDepth(GLdouble depth);



void glClearStencil(GLint s);



GLenum glClientWaitSync(GLsync sync,GLbitfield flags,GLuint64 timeout);



void glColorMask(GLboolean red,GLboolean green,GLboolean blue,GLboolean alpha);



void glColorMaski(GLuint index,GLboolean r,GLboolean g,GLboolean b,GLboolean a);



void glCompileShader(GLuint shader);



void glCompressedTexImage1D(GLenum target,GLint level,GLenum internalformat,GLsizei width,GLint border,GLsizei imageSize,const void* data);



void glCompressedTexImage2D(GLenum target,GLint level,GLenum internalformat,GLsizei width,GLsizei height,GLint border,GLsizei imageSize,const void* data);



void glCompressedTexImage3D(GLenum target,GLint level,GLenum internalformat,GLsizei width,GLsizei height,GLsizei depth,GLint border,GLsizei imageSize,const void* data);



void glCompressedTexSubImage1D(GLenum target,GLint level,GLint xoffset,GLsizei width,GLenum format,GLsizei imageSize,const void* data);



void glCompressedTexSubImage2D(GLenum target,GLint level,GLint xoffset,GLint yoffset,GLsizei width,GLsizei height,GLenum format,GLsizei imageSize,const void* data);



void glCompressedTexSubImage3D(GLenum target,GLint level,GLint xoffset,GLint yoffset,GLint zoffset,GLsizei width,GLsizei height,GLsizei depth,GLenum format,GLsizei imageSize,const void* data);



void glCopyBufferSubData(GLenum readTarget,GLenum writeTarget,GLintptr readOffset,GLintptr writeOffset,GLsizeiptr size);



void glCopyTexImage1D(GLenum target,GLint level,GLenum internalformat,GLint x,GLint y,GLsizei width,GLint border);



void glCopyTexImage2D(GLenum target,GLint level,GLenum internalformat,GLint x,GLint y,GLsizei width,GLsizei height,GLint border);



void glCopyTexSubImage1D(GLenum target,GLint level,GLint xoffset,GLint x,GLint y,GLsizei width);



void glCopyTexSubImage2D(GLenum target,GLint level,GLint xoffset,GLint yoffset,GLint x,GLint y,GLsizei width,GLsizei height);



void glCopyTexSubImage3D(GLenum target,GLint level,GLint xoffset,GLint yoffset,GLint zoffset,GLint x,GLint y,GLsizei width,GLsizei height);



GLuint glCreateProgram(void);



GLuint glCreateShader(GLenum type);



void glCullFace(GLenum mode);



void glDeleteBuffers(GLsizei n,const GLuint* buffers);



void glDeleteFramebuffers(GLsizei n,const GLuint* framebuffers);



void glDeleteProgram(GLuint program);



void glDeleteQueries(GLsizei n,const GLuint* ids);



void glDeleteRenderbuffers(GLsizei n,const GLuint* renderbuffers);



void glDeleteSamplers(GLsizei count,const GLuint* samplers);



void glDeleteShader(GLuint shader);



void glDeleteSync(GLsync sync);



void glDeleteTextures(GLsizei n,const GLuint* textures);



void glDeleteVertexArrays(GLsizei n,const GLuint* arrays);



void glDepthFunc(GLenum func);



void glDepthMask(GLboolean flag);



void glDepthRange(GLdouble n,GLdouble f);



void glDetachShader(GLuint program,GLuint shader);



void glDisable(GLenum cap);



void glDisablei(GLenum target,GLuint index);



void glDisableVertexAttribArray(GLuint index);



void glDrawArrays(GLenum mode,GLint first,GLsizei count);



void glDrawArraysInstanced(GLenum mode,GLint first,GLsizei count,GLsizei instancecount);



void glDrawBuffer(GLenum buf);



void glDrawBuffers(GLsizei n,const GLenum* bufs);



void glDrawElements(GLenum mode,GLsizei count,GLenum type,const void* indices);



void glDrawElementsBaseVertex(GLenum mode,GLsizei count,GLenum type,const void* indices,GLint basevertex);



void glDrawElementsInstanced(GLenum mode,GLsizei count,GLenum type,const void* indices,GLsizei instancecount);



void glDrawElementsInstancedBaseVertex(GLenum mode,GLsizei count,GLenum type,const void* indices,GLsizei instancecount,GLint basevertex);



void glDrawRangeElements(GLenum mode,GLuint start,GLuint end,GLsizei count,GLenum type,const void* indices);



void glDrawRangeElementsBaseVertex(GLenum mode,GLuint start,GLuint end,GLsizei count,GLenum type,const void* indices,GLint basevertex);



void glEnable(GLenum cap);



void glEnablei(GLenum target,GLuint index);



void glEnableVertexAttribArray(GLuint index);



void glEndConditionalRender(void);



void glEndQuery(GLenum target);



void glEndTransformFeedback(void);



GLsync glFenceSync(GLenum condition,GLbitfield flags);



void glFinish(void);



void glFlush(void);



void glFlushMappedBufferRange(GLenum target,GLintptr offset,GLsizeiptr length);



void glFramebufferRenderbuffer(GLenum target,GLenum attachment,GLenum renderbuffertarget,GLuint renderbuffer);



void glFramebufferTexture(GLenum target,GLenum attachment,GLuint texture,GLint level);



void glFramebufferTexture1D(GLenum target,GLenum attachment,GLenum textarget,GLuint texture,GLint level);



void glFramebufferTexture2D(GLenum target,GLenum attachment,GLenum textarget,GLuint texture,GLint level);



void glFramebufferTexture3D(GLenum target,GLenum attachment,GLenum textarget,GLuint texture,GLint level,GLint zoffset);



void glFramebufferTextureLayer(GLenum target,GLenum attachment,GLuint texture,GLint level,GLint layer);



void glFrontFace(GLenum mode);



void glGenBuffers(GLsizei n,GLuint* buffers);



void glGenerateMipmap(GLenum target);



void glGenFramebuffers(GLsizei n,GLuint* framebuffers);



void glGenQueries(GLsizei n,GLuint* ids);



void glGenRenderbuffers(GLsizei n,GLuint* renderbuffers);



void glGenSamplers(GLsizei count,GLuint* samplers);



void glGenTextures(GLsizei n,GLuint* textures);



void glGenVertexArrays(GLsizei n,GLuint* arrays);



void glGetActiveAttrib(GLuint program,GLuint index,GLsizei bufSize,GLsizei* length,GLint* size,GLenum* type,GLchar* name);



void glGetActiveUniform(GLuint program,GLuint index,GLsizei bufSize,GLsizei* length,GLint* size,GLenum* type,GLchar* name);



void glGetActiveUniformBlockiv(GLuint program,GLuint uniformBlockIndex,GLenum pname,GLint* params);



void glGetActiveUniformBlockName(GLuint program,GLuint uniformBlockIndex,GLsizei bufSize,GLsizei* length,GLchar* uniformBlockName);



void glGetActiveUniformName(GLuint program,GLuint uniformIndex,GLsizei bufSize,GLsizei* length,GLchar* uniformName);



void glGetActiveUniformsiv(GLuint program,GLsizei uniformCount,const GLuint* uniformIndices,GLenum pname,GLint* params);



void glGetAttachedShaders(GLuint program,GLsizei maxCount,GLsizei* count,GLuint* shaders);



GLint glGetAttribLocation(GLuint program,const GLchar* name);



void glGetBooleani_v(GLenum target,GLuint index,GLboolean* data);



void glGetBooleanv(GLenum pname,GLboolean* data);



void glGetBufferParameteri64v(GLenum target,GLenum pname,GLint64* params);



void glGetBufferParameteriv(GLenum target,GLenum pname,GLint* params);



void glGetBufferPointerv(GLenum target,GLenum pname,void* *params);



void glGetBufferSubData(GLenum target,GLintptr offset,GLsizeiptr size,void* data);



void glGetCompressedTexImage(GLenum target,GLint level,void* img);



void glGetDoublev(GLenum pname,GLdouble* data);



GLenum glGetError(void);



void glGetFloatv(GLenum pname,GLfloat* data);



GLint glGetFragDataIndex(GLuint program,const GLchar* name);



GLint glGetFragDataLocation(GLuint program,const GLchar* name);



void glGetFramebufferAttachmentParameteriv(GLenum target,GLenum attachment,GLenum pname,GLint* params);



void glGetInteger64i_v(GLenum target,GLuint index,GLint64* data);



void glGetInteger64v(GLenum pname,GLint64* data);



void glGetIntegeri_v(GLenum target,GLuint index,GLint* data);



void glGetIntegerv(GLenum pname,GLint* data);



void glGetMultisamplefv(GLenum pname,GLuint index,GLfloat* val);



void glGetPointerv(GLenum pname,void* *params);



void glGetProgramInfoLog(GLuint program,GLsizei bufSize,GLsizei* length,GLchar* infoLog);



void glGetProgramiv(GLuint program,GLenum pname,GLint* params);



void glGetQueryiv(GLenum target,GLenum pname,GLint* params);



void glGetQueryObjecti64v(GLuint id,GLenum pname,GLint64* params);



void glGetQueryObjectiv(GLuint id,GLenum pname,GLint* params);



void glGetQueryObjectui64v(GLuint id,GLenum pname,GLuint64* params);



void glGetQueryObjectuiv(GLuint id,GLenum pname,GLuint* params);



void glGetRenderbufferParameteriv(GLenum target,GLenum pname,GLint* params);



void glGetSamplerParameterfv(GLuint sampler,GLenum pname,GLfloat* params);



void glGetSamplerParameterIiv(GLuint sampler,GLenum pname,GLint* params);



void glGetSamplerParameterIuiv(GLuint sampler,GLenum pname,GLuint* params);



void glGetSamplerParameteriv(GLuint sampler,GLenum pname,GLint* params);



void glGetShaderInfoLog(GLuint shader,GLsizei bufSize,GLsizei* length,GLchar* infoLog);



void glGetShaderiv(GLuint shader,GLenum pname,GLint* params);



void glGetShaderSource(GLuint shader,GLsizei bufSize,GLsizei* length,GLchar* source);



const GLubyte* glGetString(GLenum name);



const GLubyte* glGetStringi(GLenum name,GLuint index);



void glGetSynciv(GLsync sync,GLenum pname,GLsizei count,GLsizei* length,GLint* values);



void glGetTexImage(GLenum target,GLint level,GLenum format,GLenum type,void* pixels);



void glGetTexLevelParameterfv(GLenum target,GLint level,GLenum pname,GLfloat* params);



void glGetTexLevelParameteriv(GLenum target,GLint level,GLenum pname,GLint* params);



void glGetTexParameterfv(GLenum target,GLenum pname,GLfloat* params);



void glGetTexParameterIiv(GLenum target,GLenum pname,GLint* params);



void glGetTexParameterIuiv(GLenum target,GLenum pname,GLuint* params);



void glGetTexParameteriv(GLenum target,GLenum pname,GLint* params);



void glGetTransformFeedbackVarying(GLuint program,GLuint index,GLsizei bufSize,GLsizei* length,GLsizei* size,GLenum* type,GLchar* name);



GLuint glGetUniformBlockIndex(GLuint program,const GLchar* uniformBlockName);



void glGetUniformfv(GLuint program,GLint location,GLfloat* params);



void glGetUniformIndices(GLuint program,GLsizei uniformCount,const GLchar* const*uniformNames,GLuint* uniformIndices);



void glGetUniformiv(GLuint program,GLint location,GLint* params);



GLint glGetUniformLocation(GLuint program,const GLchar* name);



void glGetUniformuiv(GLuint program,GLint location,GLuint* params);



void glGetVertexAttribdv(GLuint index,GLenum pname,GLdouble* params);



void glGetVertexAttribfv(GLuint index,GLenum pname,GLfloat* params);



void glGetVertexAttribIiv(GLuint index,GLenum pname,GLint* params);



void glGetVertexAttribIuiv(GLuint index,GLenum pname,GLuint* params);



void glGetVertexAttribiv(GLuint index,GLenum pname,GLint* params);



void glGetVertexAttribPointerv(GLuint index,GLenum pname,void* *pointer);



void glHint(GLenum target,GLenum mode);



GLboolean glIsBuffer(GLuint buffer);



GLboolean glIsEnabled(GLenum cap);



GLboolean glIsEnabledi(GLenum target,GLuint index);



GLboolean glIsFramebuffer(GLuint framebuffer);



GLboolean glIsProgram(GLuint program);



GLboolean glIsQuery(GLuint id);



GLboolean glIsRenderbuffer(GLuint renderbuffer);



GLboolean glIsSampler(GLuint sampler);



GLboolean glIsShader(GLuint shader);



GLboolean glIsSync(GLsync sync);



GLboolean glIsTexture(GLuint texture);



GLboolean glIsVertexArray(GLuint array);



void glLineWidth(GLfloat width);



void glLinkProgram(GLuint program);



void glLogicOp(GLenum opcode);



void* glMapBuffer(GLenum target,GLenum access);



void* glMapBufferRange(GLenum target,GLintptr offset,GLsizeiptr length,GLbitfield access);



void glMultiDrawArrays(GLenum mode,const GLint* first,const GLsizei* count,GLsizei drawcount);



void glMultiDrawElements(GLenum mode,const GLsizei* count,GLenum type,const void* const*indices,GLsizei drawcount);



void glMultiDrawElementsBaseVertex(GLenum mode,const GLsizei* count,GLenum type,const void* const*indices,GLsizei drawcount,const GLint* basevertex);



void glPixelStoref(GLenum pname,GLfloat param);



void glPixelStorei(GLenum pname,GLint param);



void glPointParameterf(GLenum pname,GLfloat param);



void glPointParameterfv(GLenum pname,const GLfloat* params);



void glPointParameteri(GLenum pname,GLint param);



void glPointParameteriv(GLenum pname,const GLint* params);



void glPointSize(GLfloat size);



void glPolygonMode(GLenum face,GLenum mode);



void glPolygonOffset(GLfloat factor,GLfloat units);



void glPrimitiveRestartIndex(GLuint index);



void glProvokingVertex(GLenum mode);



void glQueryCounter(GLuint id,GLenum target);



void glReadBuffer(GLenum src);



void glReadPixels(GLint x,GLint y,GLsizei width,GLsizei height,GLenum format,GLenum type,void* pixels);



void glRenderbufferStorage(GLenum target,GLenum internalformat,GLsizei width,GLsizei height);



void glRenderbufferStorageMultisample(GLenum target,GLsizei samples,GLenum internalformat,GLsizei width,GLsizei height);



void glSampleCoverage(GLfloat value,GLboolean invert);



void glSampleMaski(GLuint maskNumber,GLbitfield mask);



void glSamplerParameterf(GLuint sampler,GLenum pname,GLfloat param);



void glSamplerParameterfv(GLuint sampler,GLenum pname,const GLfloat* param);



void glSamplerParameteri(GLuint sampler,GLenum pname,GLint param);



void glSamplerParameterIiv(GLuint sampler,GLenum pname,const GLint* param);



void glSamplerParameterIuiv(GLuint sampler,GLenum pname,const GLuint* param);



void glSamplerParameteriv(GLuint sampler,GLenum pname,const GLint* param);



void glScissor(GLint x,GLint y,GLsizei width,GLsizei height);



void glShaderSource(GLuint shader,GLsizei count,const GLchar* const*string,const GLint* length);



void glStencilFunc(GLenum func,GLint ref,GLuint mask);



void glStencilFuncSeparate(GLenum face,GLenum func,GLint ref,GLuint mask);



void glStencilMask(GLuint mask);



void glStencilMaskSeparate(GLenum face,GLuint mask);



void glStencilOp(GLenum fail,GLenum zfail,GLenum zpass);



void glStencilOpSeparate(GLenum face,GLenum sfail,GLenum dpfail,GLenum dppass);



void glTexBuffer(GLenum target,GLenum internalformat,GLuint buffer);



void glTexImage1D(GLenum target,GLint level,GLint internalformat,GLsizei width,GLint border,GLenum format,GLenum type,const void* pixels);



void glTexImage2D(GLenum target,GLint level,GLint internalformat,GLsizei width,GLsizei height,GLint border,GLenum format,GLenum type,const void* pixels);



void glTexImage2DMultisample(GLenum target,GLsizei samples,GLenum internalformat,GLsizei width,GLsizei height,GLboolean fixedsamplelocations);



void glTexImage3D(GLenum target,GLint level,GLint internalformat,GLsizei width,GLsizei height,GLsizei depth,GLint border,GLenum format,GLenum type,const void* pixels);



void glTexImage3DMultisample(GLenum target,GLsizei samples,GLenum internalformat,GLsizei width,GLsizei height,GLsizei depth,GLboolean fixedsamplelocations);



void glTexParameterf(GLenum target,GLenum pname,GLfloat param);



void glTexParameterfv(GLenum target,GLenum pname,const GLfloat* params);



void glTexParameteri(GLenum target,GLenum pname,GLint param);



void glTexParameterIiv(GLenum target,GLenum pname,const GLint* params);



void glTexParameterIuiv(GLenum target,GLenum pname,const GLuint* params);



void glTexParameteriv(GLenum target,GLenum pname,const GLint* params);



void glTexSubImage1D(GLenum target,GLint level,GLint xoffset,GLsizei width,GLenum format,GLenum type,const void* pixels);



void glTexSubImage2D(GLenum target,GLint level,GLint xoffset,GLint yoffset,GLsizei width,GLsizei height,GLenum format,GLenum type,const void* pixels);



void glTexSubImage3D(GLenum target,GLint level,GLint xoffset,GLint yoffset,GLint zoffset,GLsizei width,GLsizei height,GLsizei depth,GLenum format,GLenum type,const void* pixels);



void glTransformFeedbackVaryings(GLuint program,GLsizei count,const GLchar* const*varyings,GLenum bufferMode);



void glUniform1f(GLint location,GLfloat v0);



void glUniform1fv(GLint location,GLsizei count,const GLfloat* value);



void glUniform1i(GLint location,GLint v0);



void glUniform1iv(GLint location,GLsizei count,const GLint* value);



void glUniform1ui(GLint location,GLuint v0);



void glUniform1uiv(GLint location,GLsizei count,const GLuint* value);



void glUniform2f(GLint location,GLfloat v0,GLfloat v1);



void glUniform2fv(GLint location,GLsizei count,const GLfloat* value);



void glUniform2i(GLint location,GLint v0,GLint v1);



void glUniform2iv(GLint location,GLsizei count,const GLint* value);



void glUniform2ui(GLint location,GLuint v0,GLuint v1);



void glUniform2uiv(GLint location,GLsizei count,const GLuint* value);



void glUniform3f(GLint location,GLfloat v0,GLfloat v1,GLfloat v2);



void glUniform3fv(GLint location,GLsizei count,const GLfloat* value);



void glUniform3i(GLint location,GLint v0,GLint v1,GLint v2);



void glUniform3iv(GLint location,GLsizei count,const GLint* value);



void glUniform3ui(GLint location,GLuint v0,GLuint v1,GLuint v2);



void glUniform3uiv(GLint location,GLsizei count,const GLuint* value);



void glUniform4f(GLint location,GLfloat v0,GLfloat v1,GLfloat v2,GLfloat v3);



void glUniform4fv(GLint location,GLsizei count,const GLfloat* value);



void glUniform4i(GLint location,GLint v0,GLint v1,GLint v2,GLint v3);



void glUniform4iv(GLint location,GLsizei count,const GLint* value);



void glUniform4ui(GLint location,GLuint v0,GLuint v1,GLuint v2,GLuint v3);



void glUniform4uiv(GLint location,GLsizei count,const GLuint* value);



void glUniformBlockBinding(GLuint program,GLuint uniformBlockIndex,GLuint uniformBlockBinding);



void glUniformMatrix2fv(GLint location,GLsizei count,GLboolean transpose,const GLfloat* value);



void glUniformMatrix2x3fv(GLint location,GLsizei count,GLboolean transpose,const GLfloat* value);



void glUniformMatrix2x4fv(GLint location,GLsizei count,GLboolean transpose,const GLfloat* value);



void glUniformMatrix3fv(GLint location,GLsizei count,GLboolean transpose,const GLfloat* value);



void glUniformMatrix3x2fv(GLint location,GLsizei count,GLboolean transpose,const GLfloat* value);



void glUniformMatrix3x4fv(GLint location,GLsizei count,GLboolean transpose,const GLfloat* value);



void glUniformMatrix4fv(GLint location,GLsizei count,GLboolean transpose,const GLfloat* value);



void glUniformMatrix4x2fv(GLint location,GLsizei count,GLboolean transpose,const GLfloat* value);



void glUniformMatrix4x3fv(GLint location,GLsizei count,GLboolean transpose,const GLfloat* value);



GLboolean glUnmapBuffer(GLenum target);



void glUseProgram(GLuint program);



void glValidateProgram(GLuint program);



void glVertexAttrib1d(GLuint index,GLdouble x);



void glVertexAttrib1dv(GLuint index,const GLdouble* v);



void glVertexAttrib1f(GLuint index,GLfloat x);



void glVertexAttrib1fv(GLuint index,const GLfloat* v);



void glVertexAttrib1s(GLuint index,GLshort x);



void glVertexAttrib1sv(GLuint index,const GLshort* v);



void glVertexAttrib2d(GLuint index,GLdouble x,GLdouble y);



void glVertexAttrib2dv(GLuint index,const GLdouble* v);



void glVertexAttrib2f(GLuint index,GLfloat x,GLfloat y);



void glVertexAttrib2fv(GLuint index,const GLfloat* v);



void glVertexAttrib2s(GLuint index,GLshort x,GLshort y);



void glVertexAttrib2sv(GLuint index,const GLshort* v);



void glVertexAttrib3d(GLuint index,GLdouble x,GLdouble y,GLdouble z);



void glVertexAttrib3dv(GLuint index,const GLdouble* v);



void glVertexAttrib3f(GLuint index,GLfloat x,GLfloat y,GLfloat z);



void glVertexAttrib3fv(GLuint index,const GLfloat* v);



void glVertexAttrib3s(GLuint index,GLshort x,GLshort y,GLshort z);



void glVertexAttrib3sv(GLuint index,const GLshort* v);



void glVertexAttrib4bv(GLuint index,const GLbyte* v);



void glVertexAttrib4d(GLuint index,GLdouble x,GLdouble y,GLdouble z,GLdouble w);



void glVertexAttrib4dv(GLuint index,const GLdouble* v);



void glVertexAttrib4f(GLuint index,GLfloat x,GLfloat y,GLfloat z,GLfloat w);



void glVertexAttrib4fv(GLuint index,const GLfloat* v);



void glVertexAttrib4iv(GLuint index,const GLint* v);



void glVertexAttrib4Nbv(GLuint index,const GLbyte* v);



void glVertexAttrib4Niv(GLuint index,const GLint* v);



void glVertexAttrib4Nsv(GLuint index,const GLshort* v);



void glVertexAttrib4Nub(GLuint index,GLubyte x,GLubyte y,GLubyte z,GLubyte w);



void glVertexAttrib4Nubv(GLuint index,const GLubyte* v);



void glVertexAttrib4Nuiv(GLuint index,const GLuint* v);



void glVertexAttrib4Nusv(GLuint index,const GLushort* v);



void glVertexAttrib4s(GLuint index,GLshort x,GLshort y,GLshort z,GLshort w);



void glVertexAttrib4sv(GLuint index,const GLshort* v);



void glVertexAttrib4ubv(GLuint index,const GLubyte* v);



void glVertexAttrib4uiv(GLuint index,const GLuint* v);



void glVertexAttrib4usv(GLuint index,const GLushort* v);



void glVertexAttribDivisor(GLuint index,GLuint divisor);



void glVertexAttribI1i(GLuint index,GLint x);



void glVertexAttribI1iv(GLuint index,const GLint* v);



void glVertexAttribI1ui(GLuint index,GLuint x);



void glVertexAttribI1uiv(GLuint index,const GLuint* v);



void glVertexAttribI2i(GLuint index,GLint x,GLint y);



void glVertexAttribI2iv(GLuint index,const GLint* v);



void glVertexAttribI2ui(GLuint index,GLuint x,GLuint y);



void glVertexAttribI2uiv(GLuint index,const GLuint* v);



void glVertexAttribI3i(GLuint index,GLint x,GLint y,GLint z);



void glVertexAttribI3iv(GLuint index,const GLint* v);



void glVertexAttribI3ui(GLuint index,GLuint x,GLuint y,GLuint z);



void glVertexAttribI3uiv(GLuint index,const GLuint* v);



void glVertexAttribI4bv(GLuint index,const GLbyte* v);



void glVertexAttribI4i(GLuint index,GLint x,GLint y,GLint z,GLint w);



void glVertexAttribI4iv(GLuint index,const GLint* v);



void glVertexAttribI4sv(GLuint index,const GLshort* v);



void glVertexAttribI4ubv(GLuint index,const GLubyte* v);



void glVertexAttribI4ui(GLuint index,GLuint x,GLuint y,GLuint z,GLuint w);



void glVertexAttribI4uiv(GLuint index,const GLuint* v);



void glVertexAttribI4usv(GLuint index,const GLushort* v);



void glVertexAttribIPointer(GLuint index,GLint size,GLenum type,GLsizei stride,const void* pointer);



void glVertexAttribP1ui(GLuint index,GLenum type,GLboolean normalized,GLuint value);



void glVertexAttribP1uiv(GLuint index,GLenum type,GLboolean normalized,const GLuint* value);



void glVertexAttribP2ui(GLuint index,GLenum type,GLboolean normalized,GLuint value);



void glVertexAttribP2uiv(GLuint index,GLenum type,GLboolean normalized,const GLuint* value);



void glVertexAttribP3ui(GLuint index,GLenum type,GLboolean normalized,GLuint value);



void glVertexAttribP3uiv(GLuint index,GLenum type,GLboolean normalized,const GLuint* value);



void glVertexAttribP4ui(GLuint index,GLenum type,GLboolean normalized,GLuint value);



void glVertexAttribP4uiv(GLuint index,GLenum type,GLboolean normalized,const GLuint* value);



void glVertexAttribPointer(GLuint index,GLint size,GLenum type,GLboolean normalized,GLsizei stride,const void* pointer);



void glViewport(GLint x,GLint y,GLsizei width,GLsizei height);



void glWaitSync(GLsync sync,GLbitfield flags,GLuint64 timeout);



#endif
