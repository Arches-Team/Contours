// Triangle

#include "triangle.h"

/*!
\brief Compute the normal vector of the triangle.

This function is expensive as it requires normalizing the cross product of the edge vectors.
\sa TriangleEdge
*/
Vector Triangle::Normal() const
{
  return Normalized((p[1] - p[0]) / (p[2] - p[0]));
}

/*!
\brief Compute the normal vector of the triangle, and scale the normal using its area.

This function is less expensive than Triangle::Normal() as it does not require normalizing the cross product of the edge vectors.

IIt is useful for computing the vertex normals of a triangle mesh by averaging the face normals of the triangles and weighting
the influence of the different triangles with their area.
\sa Triangle::Normal()
*/
Vector Triangle::AreaNormal() const
{
  return 0.5 * ((p[1] - p[0]) / (p[2] - p[0]));
}

//   // All vertices are outside the sphere. Determine the minimum squared
//   // distance between the sphere center and the triangle. 
