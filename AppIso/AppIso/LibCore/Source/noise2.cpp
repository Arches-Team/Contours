// Noise

#include "noise.h"
#include "mathematics.h"

#include "quintic.h"
#include "cubic.h"

/*
\brief Hashing function.
\param a,b Integer values
*/
inline short Noise2::Hash1d(int a, int b) const
{
  return hashTable[(int)(a) ^ ((b) & 0xfffL)];
}

inline short Noise2::Hash2d(int a, int b) const
{
  return hashTable[(int)(hashTable[(int)((a) & 0xfffL)] ^ ((b) & 0xfffL))];
}

inline double Noise2::GradientVertex(double* mp, double x, double y) const
{
  return mp[0] * 0.5 + mp[1] * x + mp[2] * y;
}

/*!
\brief Create a noise function.
\param quintic Flag defining whether quintic interpolation will be used, noise uses cubic interpolation if set to false.
*/
Noise2::Noise2(bool quintic)
{
  Noise2::quintic = quintic;
}

/*!
\brief Compute noise at a given point in the plane.

\param p Point.
*/
double Noise2::Value(const Vector2& p) const
{
  int ix = Math::Integer(p[0]);
  int iy = Math::Integer(p[1]);
  const int iz = 0;

  int jx = ix + 1;
  int jy = iy + 1;

  const double x = p[0] - ix;
  const double y = p[1] - iy;

  // Grid vertices
  int ixiy_hash = Hash2d(ix, iy);
  int jxiy_hash = Hash2d(jx, iy);
  int ixjy_hash = Hash2d(ix, jy);
  int jxjy_hash = Hash2d(jx, jy);

  int n000 = (int)Hash1d(ixiy_hash, iz) & 0xFF;
  int n100 = (int)Hash1d(jxiy_hash, iz) & 0xFF;
  int n010 = (int)Hash1d(ixjy_hash, iz) & 0xFF;
  int n110 = (int)Hash1d(jxjy_hash, iz) & 0xFF;

  // Gradients and values at grid vertices
  double g000 = GradientVertex(&rtable[n000], x, y);
  double g100 = GradientVertex(&rtable[n100], x - 1.0, y);
  double g010 = GradientVertex(&rtable[n010], x, y - 1.0);
  double g110 = GradientVertex(&rtable[n110], x - 1.0, y - 1.0);

  // Interpolation coefficients
  double sx, sy;
  if (quintic)
  {
    sx = Quintic::Smooth(x);
    sy = Quintic::Smooth(y);
  }
  else
  {
    sx = Cubic::Smooth(x);
    sy = Cubic::Smooth(y);
  }

  // The complement values
  double tx = 1.0 - sx;
  double ty = 1.0 - sy;

  // Interpolate
  double sum = (tx * ty) * g000 + (sx * ty) * g100 + (tx * sy) * g010 + (sx * sy) * g110;

  // Range at -0.5, 0.5
  sum = sum + 0.5;

  return sum;
}

