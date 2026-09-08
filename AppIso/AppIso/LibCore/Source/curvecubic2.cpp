// Curves 

#include "curve.h"

/*!
\brief Creates a cubic curve.

\param x, y %Cubic parametric equations of the corresponding x, y coordinates.
*/
CubicCurve2::CubicCurve2(const Cubic& x, const Cubic& y) :x(x), y(y)
{
}

/*!
\brief Evaluates curve point at a given location.
\param t Parameter.
*/
Vector2 CubicCurve2::operator() (const double& t) const
{
  return Vector2(x(t), y(t));
}

/*!
\brief Creates an Hermite cubic curve on interval [0,1] given
vertex locations and tangent vectors (in that order).
\param a,b End vertices of the Hermite cubic curve.
\param ta,tb Tangent vectors at the end vertices.
*/
CubicCurve2 CubicCurve2::Hermite(const Vector2& a, const Vector2& b, const Vector2& ta, const Vector2& tb)
{
  return CubicCurve2(Cubic::Hermite(a[0], b[0], ta[0], tb[0]), Cubic::Hermite(a[1], b[1], ta[1], tb[1]));
}

