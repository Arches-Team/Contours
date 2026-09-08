// Triangle

#include "triangle.h"

/*!
\brief Compute the barycenter of the triangle.
*/
Vector2 Triangle2::Center() const
{
  return (p[0] + p[1] + p[2]) / 3.0;
}

/*!
\brief Compute the area of the triangle.
\sa Triangle2::SignedArea
*/
double Triangle2::Area() const
{
  return 0.5 * Math::Abs((p[0] - p[1]) / (p[2] - p[0]));
}

/*!
\brief Compute the barycentric coordinates of an argument vector with respect to the triangle.

If the point lies within the triangle, the coordinates are positive and form a partition of the unity.
This no longer holds if the point is outside of the triangle.

Note that this function is not the most efficient, as it relies on the evaluation of the area
formed by the triangles pab, pbc, and pca.

\sa Triangle::BarycentricCoordinates()

\param p Point.
*/
Vector Triangle2::BarycentricCoordinates(const Vector2& p) const
{
  double a = Area();
  double aa = Triangle2(p, Triangle2::p[1], Triangle2::p[2]).Area();
  double ab = Triangle2(p, Triangle2::p[0], Triangle2::p[2]).Area();
  double ac = Triangle2(p, Triangle2::p[0], Triangle2::p[1]).Area();

  return Vector(aa, ab, ac) / a;
}

/*!
\brief Draw a triangle.
\param scene Graphics scene.
\param pen The pen.
\param brush The brush, should the triangle be filled.
*/
void Triangle2::Draw(QGraphicsScene& scene, const QPen& pen, const QBrush& brush) const
{
  QPolygonF polygon(3);

  // Fill with three vertices
  polygon[0] = QPointF(p[0][0], p[0][1]);
  polygon[1] = QPointF(p[1][0], p[1][1]);
  polygon[2] = QPointF(p[2][0], p[2][1]);

  scene.addPolygon(polygon, pen, brush);
}

/*!
\brief Test if a point lies inside the triangle.
\param q Point.
\author Hugo Schott
*/
bool Triangle2::Inside(const Vector2& q) const
{
  for (int i = 0; i < 3; i++)
  {
    Vector2 e = p[(i + 1) % 3] - p[i];
    Vector2 n = e.Orthogonal();
    Vector2 vecp = q - p[i];
    if ((n * vecp) < 0.)
    {
      return false;
    }
  }

  return true;
}

