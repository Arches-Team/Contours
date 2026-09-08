// Polygon

#include "polygon.h"

/*!
\brief Create a triangle.
\param a,b,c Points.
*/
Polygon2::Polygon2(const Vector2& a, const Vector2& b, const Vector2& c) :q({ a,b,c })
{
}

/*!
\brief Create a polygon.
\param p Array of points.
*/
Polygon2::Polygon2(const QVector<Vector2>& p) :q(p)
{
}

/*!
\brief Create a polygon from a set of vertexes and a subset of indexes.
\param p Array of points.
\param indexes Set of indexes.
*/
Polygon2::Polygon2(const QVector<Vector2>& p, const QVector<int>& indexes)
{
  int n = indexes.size();
  q.resize(n);

  for (int i = 0; i < n; i++)
  {
    q[i] = p.at(indexes.at(i));
  }
}

/*!
\brief Compute the bounding box of the polygon.

The polygon should have some vertices, otherwise the result is undefined.
*/
Box2 Polygon2::GetBox() const
{
  // Escape if empty
  if (q.size() == 0)
  {
    return Box2::Null;
  }

  return Box2(q);
}

/*!
\brief Compute the perimeter of the polygon.
*/
double Polygon2::Length() const
{
  if (q.size() == 0)
  {
    return 0.0;
  }

  const int n = q.size() - 1;
  double length = Norm(q.at(0) - q.at(n));

  for (int i = 0; i < n; i++)
  {
    length += Norm(q.at(i + 1) - q.at(i));
  }

  return length;
}

/*!
\brief Check if a point is inside or outside of the polygon.
\param p Point.
*/
bool Polygon2::Inside(const Vector2& p) const
{
  int n = 0;

  for (int i = 0; i < q.size(); i++)
  {
    const Vector2& a = q.at(i);
    const Vector2& b = q.at((i + 1) % q.size());

    if (p[1] > Math::Min(a[1], b[1]))
    {
      if (p[1] <= Math::Max(a[1], b[1]))
      {
        if (p[0] <= Math::Max(a[0], b[0]))
        {
          if (a[1] != b[1])
          {
            double t = (p[1] - a[1]) * (b[0] - a[0]) / (b[1] - a[1]) + a[0];
            if (a[0] == b[0] || p[0] <= t)
            {
              n++;
            }
          }
        }
      }
    }
  }

  if (n % 2 == 0)
    return false;
  else
    return true;
}

/*!
\brief Compute the signed distance between a point and a polygon.
\param p Point.
*/
double Polygon2::Signed(const Vector2& p) const
{
  double r = sqrt(RC(p));
  if (Inside(p))
  {
    return -r;
  }
  return r;
}

/*!
\brief Test if a triangle intersects the polygon.
\param t Triangle2.
*/
bool Polygon2::Intersect(const Triangle2& t) const
{
  return Intersect(Segment2(t[0], t[1])) || Intersect(Segment2(t[1], t[2])) || Intersect(Segment2(t[2], t[0]));
}

/*!
\brief Compute the squared distance between a point and the contour of the polygon.
\param p Point.
*/
double Polygon2::RC(const Vector2& p) const
{
  double r = Segment2(q.at(q.size() - 1), q.at(0)).R(p);

  for (int i = 0; i < q.size() - 1; i++)
  {
    double t = Segment2(q.at(i), q.at((i + 1))).R(p);
    if (t < r)
    {
      r = t;
    }
  }
  return r;
}

/*!
\brief Test if a segment intersects the polygon.
\param s %Segment.
*/
bool Polygon2::Intersect(const Segment2& s) const
{
  for (int i = 0; i < q.size(); i++)
  {
    Segment2 ab(q.at(i), q.at((i + 1) % q.size()));

    if (s.Intersect(ab))
    {
      return true;
    }

  }
  return false;
}

/*!
\brief Compute the (signed) area of the polygon.

The signed area can be computed by using:
\code
Polygon2 p;
double a = p.Area(true); // Signed area
\endcode

\param s Signed flag, set to false by default.
*/
double Polygon2::Area(bool s) const
{
  // Area
  double a = 0.0;

  for (int i = 0; i < q.size(); i++)
  {
    const Vector2& pa = q.at(i);
    const Vector2& pb = q.at((i + 1) % q.size());

    a += pa[0] * pb[1] - pb[0] * pa[1];
  }

  if (s == false)
  {
    a = Math::Abs(a);
  }

  return 0.5 * a;
}

/*!
\brief Compute the barycenter of the polygon.

Note that the barycenter is not the same as the centroid.
\sa Centroid
*/
Vector2 Polygon2::Center() const
{
  Vector2 g = Vector2::Null;

  for (int i = 0; i < q.size(); i++)
  {
    g += q.at(i);
  }

  return g / q.size();
}

/*!
\brief Translate the polygon by a given vector.
\param t %Vector.
*/
Polygon2 Polygon2::Translated(const Vector2& t) const
{
  QVector<Vector2> tq(q.size());

  for (int i = 0; i < q.size(); i++)
  {
    tq[i] = q.at(i) + t;
  }

  return Polygon2(tq);
}

/*!
\brief Return the scaled polygon.
\param s %Scaling vector.
*/
Polygon2 Polygon2::Scaled(const Vector2& s) const
{
  QVector<Vector2> sq(q.size());

  for (int i = 0; i < q.size(); i++)
  {
    sq[i] = q.at(i).Scaled(s);
  }

  return Polygon2(sq);
}

/*!
\brief Return the scaled polygon.
\param s %Scale.
*/
Polygon2 Polygon2::Scaled(const double& s) const
{
  return Scaled(Vector2(s));
}

/*!
\brief Scale the polygon by a given factor.
\param s Scaling factor.
*/
void Polygon2::Scale(const double& s)
{
  for (int i = 0; i < q.size(); i++)
  {
    q[i] *= s;
  }
}

/*!
\brief Convert the rectangle into a Qt polygon.
*/
QPolygonF Polygon2::GetQt() const
{
  QVector<QPointF> qp;
  for (int i = 0; i < q.size(); i++)
  {
    // Compute bits to reorder points
    Vector2 qi = q.at(i);
    qp.append(QPointF(qi[0], qi[1]));
  }

  return QPolygonF(qp);
}

/*!
\brief Draw a polygon.
\param scene Graphics scene.
\param pen The pen.
\param brush The brush.
*/
void Polygon2::Draw(QGraphicsScene& scene, const QPen& pen, const QBrush& brush) const
{
  QPolygonF polygon = GetQt();
  scene.addPolygon(polygon, pen, brush);
}

//     // If ib is above the segment ia -- ic, then the triangle is not inside but outside the polygon
//     // This is because we know the polygon points are given in trigonometric order
//     if ((q[ib] - q[ia]) / (q[ic] - q[ia]) < 0.0) continue;
