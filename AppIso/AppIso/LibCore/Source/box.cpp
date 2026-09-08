#include "box.h"

/*!
\brief Create a box given a center point and the half side length.
\param c Center.
\param r Half side length.
*/
Box::Box(const Vector& c, const double& r)
{
  a = c - Vector(r);
  b = c + Vector(r);
}

/*!
\brief Create a box given two opposite corners.

Note that this constructor does not check the coordinates of the two vectors.
Therefore, the coordinates of a should be lower than those of b.

To create the axis aligned bounding box of two vectors a and b in
the general case, one should use:
\code
Box box(Vector::Min(a,b),Vector::Max(a,b));
\endcode
\param a,b End vertices.
*/
Box::Box(const Vector& a, const Vector& b) :a(a), b(b)
{
}

/*!
\brief Create a cube centered at the origin and of given half side length.

This is equivalent to:
\code
Box box(Vector(0.0),2.0);  // Simplified constructor: Box(2.0);
\endcode
\param r Half side length.
*/
Box::Box(const double& r)
{
  a = -Vector(r);
  b = Vector(r);
}

//   // Do any of the principal axes form a separating axis ?

//   // Since the separating axis is perpendicular to the line in these
//   // last four cases, the line does not contribute to the projection
