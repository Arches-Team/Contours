// Plane

#include "plane.h"

const double Plane::epsilon = 1e-06;

/*!
\brief Compute the intersection between a plane and a ray.

The intersection depth is returned if intersection occurs.
\param ray The ray.
\param t Intersection depth.
*/
bool Plane::Intersect(const Ray& ray, double& t) const
{
  // Check if parallel
  double x = n * ray.Direction();

  if ((x < epsilon) && (x > -epsilon))
    return false;

  double y = cqc - n * ray.Origin();

  // Depth
  t = y / x;

  return true;
}

