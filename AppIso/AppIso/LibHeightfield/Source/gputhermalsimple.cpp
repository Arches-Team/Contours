#include "gpu-shader.h"
#include "gputerrainsimu.h"
#include "cpu.h"

GPUThermalErosionSimple::GPUThermalErosionSimple() {
  simulationShader = 0;

  terrain_buffer = 0;
  temp_terrain_buffer = 0;

  in_threshold_buffer = 0;
  out_threshold_buffer = 0;

  totalBufferSize = 0;
  dispatchSize = 0;

  nx = 0;
  ny = 0;
}

GPUThermalErosionSimple::~GPUThermalErosionSimple()
{
  glDeleteBuffers(1, &terrain_buffer);
  glDeleteBuffers(1, &temp_terrain_buffer);
  glDeleteBuffers(1, &in_threshold_buffer);
  glDeleteBuffers(1, &out_threshold_buffer);

  release_program(simulationShader);
}

void GPUThermalErosionSimple::Init(const HeightField& hf)
{
  // Prepare data for first step
  int nx = hf.GetSizeX();
  int ny = hf.GetSizeY();
  totalBufferSize = hf.VertexSize();
  dispatchSize = (std::max(nx, ny) / 8) + 1;

  std::vector<float> bedrock;
  bedrock.resize(totalBufferSize);
  for (int i = 0; i < totalBufferSize; i++)
    bedrock[i] = hf.at(i);

  // Prepare shader & Init buffer - Just done once
  if (simulationShader == 0) {
    QString fullPath = System::GetResource("ARCHESLIBDIR", "/LibHeightField/LibHeightField/Shaders/heightfield_thermal_simple.glsl");

    if (fullPath.isEmpty()) {
      std::cout << "GPUThermalErosionSimple::Init() : variable d'environnement ARCHESLIBDIR non d�finie" << std::endl;
      std::cin.get();
      exit(-1);
    }
    QByteArray ba = fullPath.toLocal8Bit();
    simulationShader = read_program(ba.data());
  }
  if (terrain_buffer == 0)
    glGenBuffers(1, &terrain_buffer);
  if (temp_terrain_buffer == 0)
    glGenBuffers(1, &temp_terrain_buffer);
  if (in_threshold_buffer == 0)
    glGenBuffers(1, &in_threshold_buffer);
  if (out_threshold_buffer == 0)
    glGenBuffers(1, &out_threshold_buffer);

  // Storage buffer
  glUseProgram(simulationShader);

  glBindBuffer(GL_SHADER_STORAGE_BUFFER, terrain_buffer);
  glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(float) * totalBufferSize, &bedrock.front(), GL_STREAM_READ);

  glBindBuffer(GL_SHADER_STORAGE_BUFFER, temp_terrain_buffer);
  glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(float) * totalBufferSize, &bedrock.front(), GL_STREAM_READ);

  glBindBuffer(GL_SHADER_STORAGE_BUFFER, in_threshold_buffer);
  glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(float) * totalBufferSize, &bedrock.front(), GL_STREAM_READ);

  glBindBuffer(GL_SHADER_STORAGE_BUFFER, out_threshold_buffer);
  glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(float) * totalBufferSize, &bedrock.front(), GL_STREAM_READ);

  // Uniforms - just once
  Box box = hf.GetBox();
  Vector2 cell_diag = hf.CellDiagonal();
  glUniform1i(glGetUniformLocation(simulationShader, "nx"), nx);
  glUniform1i(glGetUniformLocation(simulationShader, "ny"), ny);
  glUniform3f(glGetUniformLocation(simulationShader, "a"), box[0][0], box[0][1], box[0][2]);
  glUniform3f(glGetUniformLocation(simulationShader, "b"), box[1][0], box[1][1], box[1][2]);
  glUniform2f(glGetUniformLocation(simulationShader, "cell_diag"), cell_diag[0], cell_diag[1]);

  glUseProgram(0);

}

void GPUThermalErosionSimple::Step(int nb_steps) {

  for (int i = 0; i < nb_steps; i++) {

    glUseProgram(simulationShader);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, terrain_buffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, temp_terrain_buffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, in_threshold_buffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, out_threshold_buffer);

    glDispatchCompute(dispatchSize, dispatchSize, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    // dual buffering
    std::swap(terrain_buffer, temp_terrain_buffer);
  }

  glUseProgram(0);
}

void GPUThermalErosionSimple::SetUniforms(float eps, float tanThresholdAngle, bool noisifiedAngle, float noiseWavelength) {

  glUseProgram(simulationShader);

  glUniform1f(glGetUniformLocation(simulationShader, "eps"), eps);
  glUniform1f(glGetUniformLocation(simulationShader, "tanThresholdAngle"), tanThresholdAngle);
  glUniform1i(glGetUniformLocation(simulationShader, "noisifiedAngle"), int(noisifiedAngle));
  glUniform1f(glGetUniformLocation(simulationShader, "noiseWavelength"), noiseWavelength);

  glUseProgram(0);
}

void GPUThermalErosionSimple::GetHeight(ScalarField2& sf) {

  // Temporary float array for OpenGL.
  const int size = sf.VertexSize();
  std::vector<float> data;
  data.resize(size);

  // Get terrain
  glGetNamedBufferSubData(terrain_buffer, 0, sizeof(float) * totalBufferSize, data.data());

  for (int i = 0; i < size; i++)
    sf[i] = double(data[i]);
}

