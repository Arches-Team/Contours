// Particles

#include "particle.h"

/*!
\brief Create a set of particles.
\param r Radius.
\param p First particle.
*/
ParticleSet2::ParticleSet2(const Vector2& p, const double& r) : r(r)
{
  points.append(p);
}

/*!
\brief Get the k-th particle.
\param k Index.
*/
Circle2 ParticleSet2::At(int k) const
{
  return Circle2(points.at(k), r);
}

