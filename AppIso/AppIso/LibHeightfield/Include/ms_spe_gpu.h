#pragma once

#include "heightfield.h"
#include "cpu.h"
#include "gputerrainsimu.h"

class GPUMapAdd {
private:
  QString shader_program_name;
  GLuint shader_program_id = 0;

  GLuint coeff_buffer = 0;

  int buffer_size_x = 0;
  int buffer_size_y = 0;
  int total_buffer_size = 0;
  int dispatch_size = 0;

public:
  GPUMapAdd(const QString& _shader_program_name);
  ~GPUMapAdd();
  void Init(int _buffer_size_x, int _buffer_size_y);
  void SetCoeffMap(const ScalarField2& coeff_map);
  void Step(GLuint in, GLuint add, GLuint out);
};

class MS_SPE_GPU {
private:
  GLuint shader_program = 0;
  GPUMapProcessing processingShader = GPUMapProcessing(System::GetResource("ARCHESLIBDIR", "/LibHeightfield/LibHeightfield/Shaders/map_processing.glsl"));
  GPUMapAdd addShader = GPUMapAdd(System::GetResource("ARCHESLIBDIR", "/LibHeightfield/LibHeightfield/Shaders/map_add.glsl"));

  GLuint in_terrain_buffer = 0;
  GLuint out_terrain_buffer = 0;

  GLuint in_stream_buffer = 0;
  GLuint out_stream_buffer = 0;

  GLuint in_delta_h_buffer = 0;
  GLuint out_delta_h_buffer = 0;

  GLuint in_hardness_buffer = 0;

  int smooth_steps = 3;
  int total_buffer_size = 0;
  int dispatch_size = 0;
  int nx = 0;
  int ny = 0;

public:
  MS_SPE_GPU() {};
  ~MS_SPE_GPU();

  void Init(const HeightField& hf);
  void SetK(float k);
  void SetSmoothSteps(int s_steps) { smooth_steps = s_steps; };
  void SetHardness(const ScalarField2& hardness);
  void Step(int nb_steps);
  void GetResult(HeightField& hf);
};
