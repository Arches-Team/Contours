// Segments

#include "segment.h"
#include "triangle.h"

/*!
\brief Compute the squared distance to the segment.
\param p Point.
*/
double Segment2::R(const Vector2& p) const
{
  Vector2 n = p - b;
  // Do not normalize there
  Vector2 axis = b - a;
  double s = n * axis;

  // Squared distance 
  double r = n * n;

  // Not vertex b
  if (s < 0.0)
  {
    n = p - a;
    s = n * axis;
    r = n * n;
    // Not vertex a
    if (s > 0.0)
    {
      s = (s * s) / (axis * axis);
      r -= s;
      // Added an absolute value as imprecisions may cause rounding errors
      r = fabs(r);
    }
  }
  return r;
}

/*!
\brief Compute the squared distance to the axis edge characterized by its end points.
\param p Point.
\param s Linear coefficient corresponding to the projection of argument point p onto the edge.
*/
double Segment2::R(const Vector2& p, double& s) const
{
  Vector2 n = p - b;
  // Do not normalize there
  Vector2 axis = b - a;
  s = n * axis;

  // Squared distance 
  double r = n * n;

  // Not vertex b
  if (s < 0.0)
  {
    n = p - a;
    s = n * axis;
    r = n * n;
    // Not vertex a
    if (s > 0.0)
    {
      double length = axis * axis;
      s = s * s / length;
      r -= s; // Pythagore
      s = sqrt(s / length);
      // Added an absolute value as imprecisions may cause rounding errors
      r = fabs(r);
    }
    else
    {
      s = 0.0;
    }
  }
  else
  {
    s = 1.0;
  }
  return r;
}

/*!
\brief Test if two segments intersect.

This function computes the intersection and test the intersection parameters.
Two segments that share an end point are detected as intersecting.

\param segment Other segment.
*/
bool Segment2::Intersect(const Segment2& segment) const
{
  Vector2 u = b - a;
  Vector2 v = segment.b - segment.a;
  double d = u / v;

  // Segments are almost parallel
  if (fabs(d) < Segment2::epsilon) return false;

  Vector2 w = a - segment.a;
  double s = (v / w) / d;

  // Test if intersection point lies outside range
  if ((s < 0.0) || (s > 1.0)) return false;
  double t = (u / w) / d;
  if ((t < 0.0) || (t > 1.0)) return false;

  return true;
}

/*!
\brief Draw a segment as an arrow.
\param scene Graphics scene.
\param pen The pen.
\param brush The brush.
\param s Size of the arrow head.
*/
void Segment2::DrawArrow(QGraphicsScene& scene, const double& s, const QPen& pen, const QBrush& brush) const
{
  Vector2 u = Normalized(b - a);
  Vector2 v = u.Orthogonal();

  Vector2 c = b - u * s;

  // Arrow segment
  scene.addLine(a[0], a[1], c[0], c[1], pen);

  QPen joining = pen;
  joining.setCapStyle(Qt::RoundCap); // Could have been FlatCap, however this yields discontinuities
  joining.setJoinStyle(Qt::RoundJoin);

  // Arrow head
  Triangle2(c - 0.5 * v * s, b, c + 0.5 * v * s).Draw(scene, joining, brush);
}
