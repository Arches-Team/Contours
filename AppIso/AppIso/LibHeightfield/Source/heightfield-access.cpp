// Heightfield

#include "heightfield.h"

/*!
\brief Compute the Lipschitz constant of the signed distance field defined as f(x,y,z)=z-h(x,y).
*/
double HeightField::K() const
{
  // Lipschitz
  double lipschitz = ScalarField2::K();

  return sqrt(1.0 + lipschitz * lipschitz);
}

//   // Compute flow direction.

