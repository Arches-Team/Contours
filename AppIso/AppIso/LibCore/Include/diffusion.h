#pragma once

#include "scalarfield.h"

class Diffusion : protected Array2
{
private:
  ScalarField2 diffused; //!< Diffused field.

  ScalarField2 constraints_values;//!< Constraints.
  ScalarField2 mask; //!< Mask.

  int total_buffer_size = 0;
  int dispatch_size = 0;

  int iter_coeff = 100;

  GLuint in_diffused_buffer = 0;
  GLuint out_diffused_buffer = 0;
  GLuint in_mask_buffer = 0;
  GLuint in_constraints_buffer = 0;
  GLuint in_laplacian_buffer = 0;

  GLuint shader_program = 0;

public:
  Diffusion(const ScalarField2&, const ScalarField2&);
  ~Diffusion();

  void BuildGL();
  void CompleteVcycle();

  ScalarField2 GetResult() const { return diffused; };

protected:
  void InitBuffersGL(const ScalarField2& lap_target, const ScalarField2& start) const;
  void DispatchStepsGL(int nb_steps, int disp_size);
  void GetDataBack(ScalarField2& result) const;
  ScalarField2 UnitlessLaplacian(const ScalarField2& field) const;
  ScalarField2 VcycleStep(const ScalarField2& lap_target, const ScalarField2& start, int nb_iter);
};

