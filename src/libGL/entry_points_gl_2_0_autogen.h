// GENERATED FILE - DO NOT EDIT.
// Generated by generate_entry_points.py using data from gl.xml.
//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// entry_points_gl_2_0_autogen.h:
//   Defines the GL 2.0 entry points.

#ifndef LIBGL_ENTRY_POINTS_GL_2_0_AUTOGEN_H_
#define LIBGL_ENTRY_POINTS_GL_2_0_AUTOGEN_H_

#include <export.h>
#include "angle_gl.h"

namespace gl
{
ANGLE_EXPORT void GL_APIENTRY AttachShader(GLuint program, GLuint shader);
ANGLE_EXPORT void GL_APIENTRY BindAttribLocation(GLuint program, GLuint index, const GLchar *name);
ANGLE_EXPORT void GL_APIENTRY BlendEquationSeparate(GLenum modeRGB, GLenum modeAlpha);
ANGLE_EXPORT void GL_APIENTRY CompileShader(GLuint shader);
ANGLE_EXPORT GLuint GL_APIENTRY CreateProgram();
ANGLE_EXPORT GLuint GL_APIENTRY CreateShader(GLenum type);
ANGLE_EXPORT void GL_APIENTRY DeleteProgram(GLuint program);
ANGLE_EXPORT void GL_APIENTRY DeleteShader(GLuint shader);
ANGLE_EXPORT void GL_APIENTRY DetachShader(GLuint program, GLuint shader);
ANGLE_EXPORT void GL_APIENTRY DisableVertexAttribArray(GLuint index);
ANGLE_EXPORT void GL_APIENTRY DrawBuffers(GLsizei n, const GLenum *bufs);
ANGLE_EXPORT void GL_APIENTRY EnableVertexAttribArray(GLuint index);
ANGLE_EXPORT void GL_APIENTRY GetActiveAttrib(GLuint program,
                                              GLuint index,
                                              GLsizei bufSize,
                                              GLsizei *length,
                                              GLint *size,
                                              GLenum *type,
                                              GLchar *name);
ANGLE_EXPORT void GL_APIENTRY GetActiveUniform(GLuint program,
                                               GLuint index,
                                               GLsizei bufSize,
                                               GLsizei *length,
                                               GLint *size,
                                               GLenum *type,
                                               GLchar *name);
ANGLE_EXPORT void GL_APIENTRY GetAttachedShaders(GLuint program,
                                                 GLsizei maxCount,
                                                 GLsizei *count,
                                                 GLuint *shaders);
ANGLE_EXPORT GLint GL_APIENTRY GetAttribLocation(GLuint program, const GLchar *name);
ANGLE_EXPORT void GL_APIENTRY GetProgramInfoLog(GLuint program,
                                                GLsizei bufSize,
                                                GLsizei *length,
                                                GLchar *infoLog);
ANGLE_EXPORT void GL_APIENTRY GetProgramiv(GLuint program, GLenum pname, GLint *params);
ANGLE_EXPORT void GL_APIENTRY GetShaderInfoLog(GLuint shader,
                                               GLsizei bufSize,
                                               GLsizei *length,
                                               GLchar *infoLog);
ANGLE_EXPORT void GL_APIENTRY GetShaderSource(GLuint shader,
                                              GLsizei bufSize,
                                              GLsizei *length,
                                              GLchar *source);
ANGLE_EXPORT void GL_APIENTRY GetShaderiv(GLuint shader, GLenum pname, GLint *params);
ANGLE_EXPORT GLint GL_APIENTRY GetUniformLocation(GLuint program, const GLchar *name);
ANGLE_EXPORT void GL_APIENTRY GetUniformfv(GLuint program, GLint location, GLfloat *params);
ANGLE_EXPORT void GL_APIENTRY GetUniformiv(GLuint program, GLint location, GLint *params);
ANGLE_EXPORT void GL_APIENTRY GetVertexAttribPointerv(GLuint index, GLenum pname, void **pointer);
ANGLE_EXPORT void GL_APIENTRY GetVertexAttribdv(GLuint index, GLenum pname, GLdouble *params);
ANGLE_EXPORT void GL_APIENTRY GetVertexAttribfv(GLuint index, GLenum pname, GLfloat *params);
ANGLE_EXPORT void GL_APIENTRY GetVertexAttribiv(GLuint index, GLenum pname, GLint *params);
ANGLE_EXPORT GLboolean GL_APIENTRY IsProgram(GLuint program);
ANGLE_EXPORT GLboolean GL_APIENTRY IsShader(GLuint shader);
ANGLE_EXPORT void GL_APIENTRY LinkProgram(GLuint program);
ANGLE_EXPORT void GL_APIENTRY ShaderSource(GLuint shader,
                                           GLsizei count,
                                           const GLchar *const *string,
                                           const GLint *length);
ANGLE_EXPORT void GL_APIENTRY StencilFuncSeparate(GLenum face, GLenum func, GLint ref, GLuint mask);
ANGLE_EXPORT void GL_APIENTRY StencilMaskSeparate(GLenum face, GLuint mask);
ANGLE_EXPORT void GL_APIENTRY StencilOpSeparate(GLenum face,
                                                GLenum sfail,
                                                GLenum dpfail,
                                                GLenum dppass);
ANGLE_EXPORT void GL_APIENTRY Uniform1f(GLint location, GLfloat v0);
ANGLE_EXPORT void GL_APIENTRY Uniform1fv(GLint location, GLsizei count, const GLfloat *value);
ANGLE_EXPORT void GL_APIENTRY Uniform1i(GLint location, GLint v0);
ANGLE_EXPORT void GL_APIENTRY Uniform1iv(GLint location, GLsizei count, const GLint *value);
ANGLE_EXPORT void GL_APIENTRY Uniform2f(GLint location, GLfloat v0, GLfloat v1);
ANGLE_EXPORT void GL_APIENTRY Uniform2fv(GLint location, GLsizei count, const GLfloat *value);
ANGLE_EXPORT void GL_APIENTRY Uniform2i(GLint location, GLint v0, GLint v1);
ANGLE_EXPORT void GL_APIENTRY Uniform2iv(GLint location, GLsizei count, const GLint *value);
ANGLE_EXPORT void GL_APIENTRY Uniform3f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
ANGLE_EXPORT void GL_APIENTRY Uniform3fv(GLint location, GLsizei count, const GLfloat *value);
ANGLE_EXPORT void GL_APIENTRY Uniform3i(GLint location, GLint v0, GLint v1, GLint v2);
ANGLE_EXPORT void GL_APIENTRY Uniform3iv(GLint location, GLsizei count, const GLint *value);
ANGLE_EXPORT void GL_APIENTRY
Uniform4f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
ANGLE_EXPORT void GL_APIENTRY Uniform4fv(GLint location, GLsizei count, const GLfloat *value);
ANGLE_EXPORT void GL_APIENTRY Uniform4i(GLint location, GLint v0, GLint v1, GLint v2, GLint v3);
ANGLE_EXPORT void GL_APIENTRY Uniform4iv(GLint location, GLsizei count, const GLint *value);
ANGLE_EXPORT void GL_APIENTRY UniformMatrix2fv(GLint location,
                                               GLsizei count,
                                               GLboolean transpose,
                                               const GLfloat *value);
ANGLE_EXPORT void GL_APIENTRY UniformMatrix3fv(GLint location,
                                               GLsizei count,
                                               GLboolean transpose,
                                               const GLfloat *value);
ANGLE_EXPORT void GL_APIENTRY UniformMatrix4fv(GLint location,
                                               GLsizei count,
                                               GLboolean transpose,
                                               const GLfloat *value);
ANGLE_EXPORT void GL_APIENTRY UseProgram(GLuint program);
ANGLE_EXPORT void GL_APIENTRY ValidateProgram(GLuint program);
ANGLE_EXPORT void GL_APIENTRY VertexAttrib1d(GLuint index, GLdouble x);
ANGLE_EXPORT void GL_APIENTRY VertexAttrib1dv(GLuint index, const GLdouble *v);
ANGLE_EXPORT void GL_APIENTRY VertexAttrib1f(GLuint index, GLfloat x);
ANGLE_EXPORT void GL_APIENTRY VertexAttrib1fv(GLuint index, const GLfloat *v);
ANGLE_EXPORT void GL_APIENTRY VertexAttrib1s(GLuint index, GLshort x);
ANGLE_EXPORT void GL_APIENTRY VertexAttrib1sv(GLuint index, const GLshort *v);
ANGLE_EXPORT void GL_APIENTRY VertexAttrib2d(GLuint index, GLdouble x, GLdouble y);
ANGLE_EXPORT void GL_APIENTRY VertexAttrib2dv(GLuint index, const GLdouble *v);
ANGLE_EXPORT void GL_APIENTRY VertexAttrib2f(GLuint index, GLfloat x, GLfloat y);
ANGLE_EXPORT void GL_APIENTRY VertexAttrib2fv(GLuint index, const GLfloat *v);
ANGLE_EXPORT void GL_APIENTRY VertexAttrib2s(GLuint index, GLshort x, GLshort y);
ANGLE_EXPORT void GL_APIENTRY VertexAttrib2sv(GLuint index, const GLshort *v);
ANGLE_EXPORT void GL_APIENTRY VertexAttrib3d(GLuint index, GLdouble x, GLdouble y, GLdouble z);
ANGLE_EXPORT void GL_APIENTRY VertexAttrib3dv(GLuint index, const GLdouble *v);
ANGLE_EXPORT void GL_APIENTRY VertexAttrib3f(GLuint index, GLfloat x, GLfloat y, GLfloat z);
ANGLE_EXPORT void GL_APIENTRY VertexAttrib3fv(GLuint index, const GLfloat *v);
ANGLE_EXPORT void GL_APIENTRY VertexAttrib3s(GLuint index, GLshort x, GLshort y, GLshort z);
ANGLE_EXPORT void GL_APIENTRY VertexAttrib3sv(GLuint index, const GLshort *v);
ANGLE_EXPORT void GL_APIENTRY VertexAttrib4Nbv(GLuint index, const GLbyte *v);
ANGLE_EXPORT void GL_APIENTRY VertexAttrib4Niv(GLuint index, const GLint *v);
ANGLE_EXPORT void GL_APIENTRY VertexAttrib4Nsv(GLuint index, const GLshort *v);
ANGLE_EXPORT void GL_APIENTRY
VertexAttrib4Nub(GLuint index, GLubyte x, GLubyte y, GLubyte z, GLubyte w);
ANGLE_EXPORT void GL_APIENTRY VertexAttrib4Nubv(GLuint index, const GLubyte *v);
ANGLE_EXPORT void GL_APIENTRY VertexAttrib4Nuiv(GLuint index, const GLuint *v);
ANGLE_EXPORT void GL_APIENTRY VertexAttrib4Nusv(GLuint index, const GLushort *v);
ANGLE_EXPORT void GL_APIENTRY VertexAttrib4bv(GLuint index, const GLbyte *v);
ANGLE_EXPORT void GL_APIENTRY
VertexAttrib4d(GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
ANGLE_EXPORT void GL_APIENTRY VertexAttrib4dv(GLuint index, const GLdouble *v);
ANGLE_EXPORT void GL_APIENTRY
VertexAttrib4f(GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
ANGLE_EXPORT void GL_APIENTRY VertexAttrib4fv(GLuint index, const GLfloat *v);
ANGLE_EXPORT void GL_APIENTRY VertexAttrib4iv(GLuint index, const GLint *v);
ANGLE_EXPORT void GL_APIENTRY
VertexAttrib4s(GLuint index, GLshort x, GLshort y, GLshort z, GLshort w);
ANGLE_EXPORT void GL_APIENTRY VertexAttrib4sv(GLuint index, const GLshort *v);
ANGLE_EXPORT void GL_APIENTRY VertexAttrib4ubv(GLuint index, const GLubyte *v);
ANGLE_EXPORT void GL_APIENTRY VertexAttrib4uiv(GLuint index, const GLuint *v);
ANGLE_EXPORT void GL_APIENTRY VertexAttrib4usv(GLuint index, const GLushort *v);
ANGLE_EXPORT void GL_APIENTRY VertexAttribPointer(GLuint index,
                                                  GLint size,
                                                  GLenum type,
                                                  GLboolean normalized,
                                                  GLsizei stride,
                                                  const void *pointer);
}  // namespace gl

#endif  // LIBGL_ENTRY_POINTS_GL_2_0_AUTOGEN_H_
