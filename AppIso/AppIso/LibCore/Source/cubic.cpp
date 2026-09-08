// Cubic polynomials

#include "cubic.h"

/*!
\brief Creates an Hermite cubic polynomial.

Interval parameterization is unit.

\param a, b Values for t=0 and t=1.
\param ta, tb Derivative values for t=0 and t=1.

\sa Math::Cubic()
*/
Cubic Cubic::Hermite(const double& a, const double& b, const double& ta, const double& tb)
{
  return Cubic(tb + ta + 2.0 * (a - b), -tb - 2.0 * ta + 3.0 * (b - a), ta, a);
}

/*!
\brief Bicubic interpolation based on four values.
\param a,b,c,d Four input values.
\param x Interpolant.
\author Lois Paulin
*/
double Cubic::Interpolation(const double& x, const double& a, const double& b, const double& c, const double& d)
{
  double a_m1 = ((-0.5 * x + 1.0) * x - 0.5) * x;
  double a_0 = (1.5 * x - 2.5) * x * x + 1.0;
  double a_1 = ((-1.5 * x + 2.0) * x + 0.5) * x;
  double a_2 = (0.5 * x - 0.5) * x * x;

  return a_m1 * a + a_0 * b + a_1 * c + a_2 * d;
}

