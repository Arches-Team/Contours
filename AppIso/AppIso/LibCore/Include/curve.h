// Curve
#pragma once

#include "evector.h"
#include "cubic.h"

//   // Unary operators

// Quadric curve class
class CubicCurve2
{
protected:
  Cubic x, y; //!< %Cubic polynomial functions for every coordinate.
public:
  //! Empty.
  CubicCurve2() {}
  explicit CubicCurve2(const Cubic&, const Cubic&);

  //! Empty.
  ~CubicCurve2() {}

//   // Unary operators

  // Evaluates curve
  Vector2 operator()(const double&) const;

public:
  static CubicCurve2 Hermite(const Vector2&, const Vector2&, const Vector2&, const Vector2&);
};

