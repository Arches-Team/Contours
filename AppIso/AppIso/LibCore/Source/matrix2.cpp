// Matrix 

#include "matrix.h"

/*!
\brief Create a rotation matrix.

\param a Angle in radian.
*/
Matrix2 Matrix2::Rotation(const double& a)
{
  double c = cos(a);
  double s = sin(a);
  return Matrix2(c, s, -s, c);
}

