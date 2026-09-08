#pragma once

#include "gpu-shader.h"
#include "evector.h"

class GLBuffer
{
protected:
  GLuint buffer;
public:
  inline GLBuffer() : buffer(0) { }
  inline ~GLBuffer() { }

  void Generate();
  void Destroy();

  void SetData(int bufferType, size_t size, void* data, int hint = GL_STATIC_DRAW);
  void Bind(int bufferType) const;
  void BindAt(int bufferType, int bindIndex) const;

  GLuint GetBuffer() const;
};

inline void GLBuffer::Generate() {
  if (buffer == 0)
    glGenBuffers(1, &buffer);
}

inline void GLBuffer::Destroy() {
  glDeleteBuffers(1, &buffer);
}

inline void GLBuffer::SetData(int bufferType, size_t size, void* data, int hint) {
  Bind(bufferType);
  glBufferData(bufferType, size, data, hint);
}

inline void GLBuffer::Bind(int bufferType) const {
  glBindBuffer(bufferType, buffer);
}

inline void GLBuffer::BindAt(int bufferType, int bindIndex) const
{
  glBindBufferBase(bufferType, bindIndex, buffer);
}

inline GLuint GLBuffer::GetBuffer() const {
  return buffer;
}

class GLShader
{
protected:
  GLuint program = 0;
public:
  inline GLShader() { }
  inline ~GLShader() { }

  void Initialize(const char*);
  void Destroy();
  void Bind() const;

  void SetUniform(const char*, float, float) const;
  void SetUniform(const char*, int) const;
  void SetUniform(const char*, const Vector&) const;
  void SetUniform(const char*, const float* mat4) const;
  void SetUniform(const char*, float* mat4) const;

  void SetUniform(int index, float, float) const;
  void SetUniform(int index, const Vector&) const;

};

inline void GLShader::Initialize(const char* path) {
  program = read_program(path);
}

inline void GLShader::Destroy() {
  release_program(program);
}

inline void GLShader::Bind() const {
  glUseProgram(program);
}

inline void GLShader::SetUniform(const char* name, float v0, float v1) const {
  glUniform2f(glGetUniformLocation(program, name), v0, v1);
}

inline void GLShader::SetUniform(const char* name, int v) const {
  glUniform1i(glGetUniformLocation(program, name), v);
}

inline void GLShader::SetUniform(const char* name, const Vector& v) const {
  glUniform3f(glGetUniformLocation(program, name), float(v[0]), float(v[1]), float(v[2]));
}

inline void GLShader::SetUniform(const char* name, const float* mat4) const {
  glUniformMatrix4fv(glGetUniformLocation(program, name), 1, GL_FALSE, mat4);
}

inline void GLShader::SetUniform(const char* name, float* mat4) const {
  glUniformMatrix4fv(glGetUniformLocation(program, name), 1, GL_FALSE, mat4);
}

inline void GLShader::SetUniform(int index, float v0, float v1) const {
  glUniform2f(index, v0, v1);
}

inline void GLShader::SetUniform(int index, const Vector& v) const {
  glUniform3f(index, float(v[0]), float(v[1]), float(v[2]));
}

