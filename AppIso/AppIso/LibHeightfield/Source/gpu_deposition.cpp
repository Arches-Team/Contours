#include "gpusediment.h"

#include "gpu-shader.h"
#include "cpu.h"

/*!
\class GPU_Deposition gpusediment.h
\brief A sediment deposition simulation on the GPU. Simulation only adds matter on a given bedrock
*/

/*!
\brief Constructor.
*/
GPU_Deposition::GPU_Deposition() {
  simulationShader = 0;

  terrain_buffer = 0;
  temp_terrain_buffer = 0;

  stream_buffer = 0;
  temp_stream_buffer = 0;

  sed_buffer = 0;
  temp_sed_buffer = 0;

  source_map_buffer = 0;

  debug_buffer = 0;

  nx = 0;
  ny = 0;
}

/*!
\brief Destructor.
*/
GPU_Deposition::~GPU_Deposition() {
  glDeleteBuffers(1, &terrain_buffer);
  glDeleteBuffers(1, &temp_terrain_buffer);

  glDeleteBuffers(1, &stream_buffer);
  glDeleteBuffers(1, &temp_stream_buffer);

  glDeleteBuffers(1, &sed_buffer);
  glDeleteBuffers(1, &temp_sed_buffer);

  glDeleteBuffers(1, &source_map_buffer);

  glDeleteBuffers(1, &debug_buffer);

  release_program(simulationShader);
}

/*!
\brief GPU buffers initialization from given heightfield. Shader and Uniforms are also initialized.
\param hf Heightfield to be used as bedrock.
*/
void GPU_Deposition::Init(const HeightField& hf) {
  // Prepare data for first step
  int nx = hf.GetSizeX();
  int ny = hf.GetSizeY();
  totalBufferSize = hf.VertexSize();
  dispatchSize = (std::max(nx, ny) / 8) + 1;

  std::vector<float> bedrock;
  std::vector<float> zeros(totalBufferSize, 0.0);
  std::vector<float> ones(totalBufferSize, 1.0);
  bedrock.resize(totalBufferSize);
  for (int i = 0; i < totalBufferSize; i++)
    bedrock[i] = hf.at(i);

  // Prepare shader & Init buffer - Just done once
  if (simulationShader == 0) {
    QString fullPath = System::GetResource("ARCHESLIBDIR", "/LibHeightField/LibHeightField/Shaders/deposition.glsl");

    if (fullPath.isEmpty()) {
      std::cout << "GPU_Deposition::Init() : variable d'environnement ARCHESLIBDIR non d�finie" << std::endl;
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
  if (stream_buffer == 0)
    glGenBuffers(1, &stream_buffer);
  if (temp_stream_buffer == 0)
    glGenBuffers(1, &temp_stream_buffer);
  if (sed_buffer == 0)
    glGenBuffers(1, &sed_buffer);
  if (temp_sed_buffer == 0)
    glGenBuffers(1, &temp_sed_buffer);
  if (source_map_buffer == 0)
    glGenBuffers(1, &source_map_buffer);
  if (debug_buffer == 0)
    glGenBuffers(1, &debug_buffer);

  // Storage buffer
  glUseProgram(simulationShader);

  glBindBuffer(GL_SHADER_STORAGE_BUFFER, terrain_buffer);
  glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(float) * totalBufferSize, &bedrock.front(), GL_STREAM_READ);

  glBindBuffer(GL_SHADER_STORAGE_BUFFER, temp_terrain_buffer);
  glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(float) * totalBufferSize, &bedrock.front(), GL_STREAM_READ);

  glBindBuffer(GL_SHADER_STORAGE_BUFFER, stream_buffer);
  glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(float) * totalBufferSize, &ones.front(), GL_STREAM_READ);

  glBindBuffer(GL_SHADER_STORAGE_BUFFER, temp_stream_buffer);
  glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(float) * totalBufferSize, &zeros.front(), GL_STREAM_READ);

  glBindBuffer(GL_SHADER_STORAGE_BUFFER, sed_buffer);
  glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(float) * totalBufferSize, &zeros.front(), GL_STREAM_READ);

  glBindBuffer(GL_SHADER_STORAGE_BUFFER, temp_sed_buffer);
  glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(float) * totalBufferSize, &zeros.front(), GL_STREAM_READ);

  glBindBuffer(GL_SHADER_STORAGE_BUFFER, source_map_buffer);
  glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(float) * totalBufferSize, &ones.front(), GL_STREAM_READ);

  glBindBuffer(GL_SHADER_STORAGE_BUFFER, debug_buffer);
  glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(float) * totalBufferSize, &zeros.front(), GL_STREAM_READ);

  // Uniforms - just once
  Box box = hf.GetBox();
  Vector2 cell_diag = hf.CellDiagonal();
  glUniform1i(glGetUniformLocation(simulationShader, "nx"), nx);
  glUniform1i(glGetUniformLocation(simulationShader, "ny"), ny);
  glUniform3f(glGetUniformLocation(simulationShader, "a"), box[0][0], box[0][1], box[0][2]);
  //std::cout << box[0][0] << " " << box[0][1] << " " << box[0][2] << std::endl;
  glUniform3f(glGetUniformLocation(simulationShader, "b"), box[1][0], box[1][1], box[1][2]);
  glUniform2f(glGetUniformLocation(simulationShader, "cellDiag"), cell_diag[0], cell_diag[1]);

  glUseProgram(0);
}

/*!
\brief Launches nb_steps steps of the deposition simulation.
\param nb_steps Number of simulation steps to launch.
*/
void GPU_Deposition::Step(int nb_steps) {

  for (int i = 0; i < nb_steps; i++) {

    glUseProgram(simulationShader);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, terrain_buffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, temp_terrain_buffer);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, stream_buffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, temp_stream_buffer);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, sed_buffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, temp_sed_buffer);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, source_map_buffer);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, debug_buffer);

    glDispatchCompute(dispatchSize, dispatchSize, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    // dual buffering
    std::swap(terrain_buffer, temp_terrain_buffer);
    std::swap(stream_buffer, temp_stream_buffer);
    std::swap(sed_buffer, temp_sed_buffer);
  }

  glUseProgram(0);
}

/*!
\brief Set the uniform variable associated to deposition strength.
\param deposition_strength Deposition strength value.
*/
void GPU_Deposition::SetUniforms(float deposition_strength) {

  glUseProgram(simulationShader);

  glUniform1f(glGetUniformLocation(simulationShader, "deposition_strength"), deposition_strength);

  glUseProgram(0);
}

/*!
\brief Gets the elevation data back to the CPU.
\param sf ScalarField to store the elevation data.
*/
void GPU_Deposition::GetHeight(ScalarField2& sf) {

  // Temporary float array for OpenGL.
  const int size = sf.VertexSize();
  std::vector<float> data;
  data.resize(size);

  // Get terrain
  glGetNamedBufferSubData(terrain_buffer, 0, sizeof(float) * totalBufferSize, data.data());

  for (int i = 0; i < size; i++)
    sf[i] = double(data[i]);
}

// //EOF