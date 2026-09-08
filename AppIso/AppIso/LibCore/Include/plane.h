// Plane

#pragma once

#include "evector.h"
#include "ray.h"

// Plane 
class Plane
{
protected:
  Vector n = Vector::Z;  //!< %Plane normal.
  double cqc = 0.0;  //!< %Plane coefficient.
public:
  //! Empty.
  Plane() {}
  explicit Plane(const Vector&, const double&);

  bool Intersect(const Ray&, double&) const;

public:
  static const double epsilon; //!< Small value for checking intersection with a ray.
};

/*!
\brief Creates a plane given normal and distance.

The equation of the plane is: \f$\ep \cdot \en - c = 0\f$.

\param n %Normal, should be unit.
\param c %Plane constant.
*/
inline Plane::Plane(const Vector& n, const double& c) :n(n), cqc(c)
{
}

