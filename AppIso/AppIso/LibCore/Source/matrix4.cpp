// Matrix

#include "matrix.h"

/*!
\brief Create an homogeneous matrix from a simple Matrix and a translation vector

The shear coefficients are set to 0.0.
\param a Matrix.
\param t Translation vector.
*/
Matrix4::Matrix4(const Matrix& a, const Vector& t) :Matrix4(a, t, Vector::Null)
{
}

/*!
\brief Constructor from a Matrix, a translation and a shear vector.
\param a Matrix.
\param t Translation vector.
\param s Shear vector.
*/
Matrix4::Matrix4(const Matrix& a, const Vector& t, const Vector& s)
{
  // Rotation and scale
  r[0] = a[0];
  r[1] = a[1];
  r[2] = a[2];
  r[4] = a[3];
  r[5] = a[4];
  r[6] = a[5];
  r[8] = a[6];
  r[9] = a[7];
  r[10] = a[8];

  // Translation
  r[12] = t[0];
  r[13] = t[1];
  r[14] = t[2];

  // Shear
  r[3] = s[0];
  r[7] = s[1];
  r[11] = s[2];

  // Scale
  r[15] = 1.0;
}

/*!
\brief Convert to a generic 4&times;4 float matrix.
\param a Returned matrix.
*/
void Matrix4::Float(float a[16]) const
{
  for (int i = 0; i < 16; i++)
  {
    a[i] = float(r[i]);
  }
}
