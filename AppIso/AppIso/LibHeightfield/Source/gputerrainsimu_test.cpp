#include "gputerrainsimu.h"
#include "gpu-shader.h"

// GPUMapProcessing ----------------------------------------------------------
GPUMapProcessing::GPUMapProcessing(const QString& _shader_program_name) {
  shader_program_name = _shader_program_name;
}

GPUMapProcessing::~GPUMapProcessing() {
  /*glDeleteBuffers(1, &outBuffer);
  glDeleteBuffers(1, &tempBuffer);*/

  release_program(shader_program_id);
}

void GPUMapProcessing::Init(int _buffer_size_x, int _buffer_size_y, float _cellSize) {
  buffer_size_x = _buffer_size_x;
  buffer_size_y = _buffer_size_y;
  cellSize = _cellSize;
  total_buffer_size = buffer_size_x * buffer_size_y;
  dispatch_size = (std::max(buffer_size_x, buffer_size_x) / 8) + 1;

  //std::vector<float> tmpZeroes(total_buffer_size, 0);

  // Prepare shader & Init buffer - Just done once
  if (shader_program_id == 0) {
    QString fullPath = shader_program_name;
    QByteArray ba = fullPath.toLocal8Bit();
    shader_program_id = read_program(ba.data());
  }

  /*glGenBuffers(1, &outBuffer);
  glGenBuffers(1, &tempBuffer);*/

  // Storage buffer
  glUseProgram(shader_program_id);

  /*glBindBuffer(GL_SHADER_STORAGE_BUFFER, outBuffer);
  glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(float) * total_buffer_size, &tmpZeroes.front(), GL_STREAM_READ);

  glBindBuffer(GL_SHADER_STORAGE_BUFFER, tempBuffer);
  glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(float) * total_buffer_size, &tmpZeroes.front(), GL_STREAM_READ);

  glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);*/

  // Uniforms - just once
  glUniform1i(glGetUniformLocation(shader_program_id, "buffer_size_x"), buffer_size_x);
  glUniform1i(glGetUniformLocation(shader_program_id, "buffer_size_y"), buffer_size_y);

  glUseProgram(0);
}

void GPUMapProcessing::Step(int smooth_steps, GLuint& in, GLuint& out) {
  glUseProgram(shader_program_id);

  for (int i = 0; i < smooth_steps; i++) {

    //glUniform1f(glGetUniformLocation(shader_program_id, "threshold"), threshold);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, out);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, in);

    glDispatchCompute(dispatch_size, dispatch_size, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    std::swap(in, out);
  }

  std::swap(in, out);

  glUseProgram(0);
}