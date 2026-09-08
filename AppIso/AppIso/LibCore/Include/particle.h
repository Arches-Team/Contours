// Fundamentals

#pragma once

#include "circle.h"

class ParticleSet2
{ 
protected:
  QVector<Vector2> points; //!< Set of particles
  double r = 0.0; //!< Radius of particles.
public:
  ParticleSet2() : r(1.0) {}
  explicit ParticleSet2(const Vector2&, const double&);

  int Size() const;

  Circle2 At(int) const;
  double Radius() const;

};

/*!
\brief Return the radius of the particles.
*/
inline double ParticleSet2::Radius() const
{
  return r;
}

/*!
\brief Return the number of particles.
*/
inline int ParticleSet2::Size() const
{
  return points.size();
}
