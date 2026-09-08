#pragma once

#include "heightfield.h"

class GPUMapProcessing {
private:

  QString shader_program_name;
  GLuint shader_program_id = 0;

  int buffer_size_x = 0;
  int buffer_size_y = 0;
  float cellSize;
  int total_buffer_size = 0;
  int dispatch_size = 0;

public:
  GPUMapProcessing(const QString& _shader_program_name);
  ~GPUMapProcessing();

  void Init(int _buffer_size_x, int _buffer_size_y, float _cellSize);
  void Step(int smooth_steps, GLuint& in, GLuint& out);

};

class GPUThermalErosionSimple
{
protected:
  GLuint simulationShader;	//!< Compute shader.

  GLuint terrain_buffer;
  GLuint temp_terrain_buffer;

  GLuint in_threshold_buffer;
  GLuint out_threshold_buffer;

  int nx, ny;
  int totalBufferSize;			//!< Total buffer size defined as nx * ny
  int dispatchSize;				//!< Single dispatch size

public:
  GPUThermalErosionSimple();
  ~GPUThermalErosionSimple();

  void Init(const HeightField&);
  void Step(int);
  void SetUniforms(float eps, float tanThresholdAngle, bool noisifiedAngle, float noiseWavelength);
  void GetHeight(ScalarField2& hf);
};

