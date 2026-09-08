
#include "box.h"

const double Box2::epsilon = 1.0e-5; //!< Epsilon value used to check intersections and some round off errors.

const Box2 Box2::Null(0.0); //!< Null box, equivalent to \code Box2(Vector2(0.0)); \endcode

/*!
\brief Compute the bounding box of a set of points.
\param p Set of points (should not be empty).
*/
Box2::Box2(const QVector<Vector2>& p)
{
  a = p.at(0);
  b = p.at(0);
  for (int i = 1; i < p.size(); i++)
  {
    a = Vector2::Min(a, p.at(i));
    b = Vector2::Max(b, p.at(i));
  }
}

/*!
\brief Create a box embedding two boxes.
\param x,y Argument boxes.
*/
Box2::Box2(const Box2& x, const Box2& y)
{
  a = Vector2::Min(x.a, y.a);
  b = Vector2::Max(x.b, y.b);
}

/*!
\brief Create a box from a Qt size.
\param size Size.
*/
Box2::Box2(const QSize& size)
{
  a = Vector2::Null;
  b = Vector2(size.width(), size.height());
}

/*!
\brief Extend the limits of the box by a given distance.

Note that this is the same as performing the Minkowski sum with a cubic box of size r.
\param r Range.
*/
Box2 Box2::Extended(const double& r) const
{
  return Box2(a - Vector2(r), b + Vector2(r));
}

/*!
\brief Test if a point is inside the box.
\param p Point.
*/
bool Box2::Inside(const Vector2& p) const
{
  if ((p[0] < a[0]) || (p[0] > b[0]) || (p[1] < a[1]) || (p[1] > b[1]))
    return false;
  else
    return true;
}

/*!
\brief Convert a planar Box2 to a Box.
\param a,b Lower and upper coordinates (note that a should be smaller than b).
*/
Box Box2::ToBox(const double& a, const double& b) const
{
  return Box(Box2::a.ToVector(a), Box2::b.ToVector(b));
}

/*!
\brief Translated box.

\param t Translation vector.
*/
Box2 Box2::Translated(const Vector2& t) const
{
  return Box2(a + t, b + t);
}

/*!
\brief Scales a box.

Note that this function handles negative coefficients in
the scaling vector.
\param s Scaling vector.
*/
void Box2::Scale(const Vector2& s)
{
  a *= s;
  b *= s;
  // Swap coordinates for negative coefficients 
  for (int i = 0; i < 2; i++)
  {
    if (s[i] < 0.0)
    {
      Math::Swap(a[i], b[i]);
    }
  }
}

/*!
\brief Scales a box.

Note that this function handles negative coefficients.
\param s Scaling factor.
*/
void Box2::Scale(const double& s)
{
  Box2::Scale(Vector2(s));
}

/*!
\brief Scales a box and return the scaled box.

\param s Scaling factor.
\sa Box2::Scaled(const Vector2&)
*/
Box2 Box2::Scaled(const double& s) const
{
  if (s > 0.0)
  {
    return Box2(a * s, b * s);
  }
  else
  {
    return Box2(b * s, a * s);
  }
}

/*!
\brief Scales a box while preserving its center.

\param s Size.
\sa Box2::Scaled
*/
Box2 Box2::ScaledCentered(const double& s) const
{
  Vector2 c = 0.5 * (a + b);
  Vector2 d = 0.5 * s * (b - a);
  return Box2(c - d, c + d);
}

/*!
\brief Draw a rectangle.
\param scene Graphics scene.
\param pen The pen.
\param brush The brush, should the box be filled.
*/
void Box2::Draw(QGraphicsScene& scene, const QPen& pen, const QBrush& brush) const
{
  scene.addRect(a[0], a[1], b[0] - a[0], b[1] - a[1], pen, brush);
}

/*!
\brief Create the Qt rectangle.
*/
QRectF Box2::GetQtRect() const
{
  return QRectF(a[0], a[1], b[0] - a[0], b[1] - a[1]);
}

//   // Integer starting point

/*!
\brief Return the tiled the box using integer coordinates.
\param x,y Integer coordinates.
\sa TileRange(const Box2&), Tile(const QRect&)
*/
Box2 Box2::Tile(int x, int y) const
{
  // Translation vector
  Vector2 t = (b - a).Scaled(Vector2(x, y));

  return Translated(t);
}

