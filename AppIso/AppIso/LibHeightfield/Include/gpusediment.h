#pragma once

#include "heightfield.h"

class GPU_Deposition {
private:

  GLuint simulationShader;

  GLuint terrain_buffer;
  GLuint temp_terrain_buffer;

  GLuint stream_buffer;
  GLuint temp_stream_buffer;

  GLuint sed_buffer;
  GLuint temp_sed_buffer;

  GLuint source_map_buffer;

  GLuint debug_buffer;

  int nx, ny;
  int totalBufferSize = 0;
  int dispatchSize = 0;

public:
  GPU_Deposition();
  ~GPU_Deposition();

  void Init(const HeightField&);
  void Step(int);
  void SetUniforms(float deposition_strength);
  void GetHeight(ScalarField2& hf);
};

