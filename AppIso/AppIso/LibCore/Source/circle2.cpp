// Circle

#include "circle.h"

/*!
\class Circle2 circle.h
\brief Circles in the plane.

\sa Circle
\ingroup PlanarGroup
*/

/*!
\brief Create a circle.
\param c Center.
\param r Radius.
*/
Circle2::Circle2(const Vector2& c, const double& r) : c(c), r(r)
{
}

/*!
\brief Create the circle passing through 3 points.

This is the circumscribed circle of the triangle.

\param a, b, c Points.
*/
Circle2::Circle2(const Vector2& a, const Vector2& b, const Vector2& c)
{
  Vector2 ab = b - a;
  Vector2 ac = c - a;
  Vector2 bc = c - b;

  double E = ab[0] * (a[0] + b[0]) + ab[1] * (a[1] + b[1]);
  double F = ac[0] * (a[0] + c[0]) + ac[1] * (a[1] + c[1]);

  double G = 2.0 * (ab[0] * bc[1] - ab[1] * bc[0]);

  double x = (ac[1] * E - ab[1] * F) / G;
  double y = (ab[0] * F - ac[0] * E) / G;

  Circle2::c = Vector2(x, y);
  Circle2::r = Norm(a - Circle2::c);
}

/*!
\brief Check if a point is inside the circle.
\param p Point.
*/
bool Circle2::Inside(const Vector2& p) const
{
  if (SquaredNorm(p - c) <= r * r)
  {
    return true;
  }
  else
  {
    return false;
  }
}

/*!
\brief Draw a circle.
\param scene Graphics scene.
\param pen The pen.
\param brush The brush, should the circle be filled.
*/
void Circle2::Draw(QGraphicsScene& scene, const QPen& pen, const QBrush& brush) const
{
  scene.addEllipse(c[0] - r, c[1] - r, r + r, r + r, pen, brush);
}

/*!
\brief Extend the circle so that the argument point should be embedded in the new circle.

This is the same as computing the smallest enclosing circle of a circle and a point.

\param p Point.
*/
void Circle2::Extend(const Vector2& p)
{
  Vector2 pc = p - c;

  // Squared distance
  double re = pc * pc;

  // Escape if point lies inside the circle
  if (re < r * r)
    return;

  // Distance
  re = sqrt(re);

  // Normalized direction
  pc /= re;

  // Compute half difference
  re = 0.5 * (re - r);

  // Move center by half difference
  c += pc * re;

  r += re;
}

/*!
\brief Check the intersection between a circle and a ray.

Note that intersections are sorted.

This function assumes that the ray is normalized, i.e. has a unit direction vector.

\param ray The (normalized) ray.
\param ta, tb Intersection depths.
*/
bool Circle2::Intersect(const Ray2& ray, double& ta, double& tb) const
{
  // Center
  Vector2 n = c - ray.Origin();

  // Distance to center
  double k = n * n;
  double t = n * ray.Direction();
  if (!(k < r * r) && (t < Epsilon))
    return false;

  double h = r * r - k + t * t;

  if (h < Epsilon)
    return false;

  // Intersection depths
  h = sqrt(h);
  ta = t - h;
  tb = t + h;

  return true;
}

