// Vector sets

#include "vectorset.h"

/*!
\brief Create a set of vectors.
\param s The set.
*/
VectorSet2::VectorSet2(const QVector<Vector2>& s) :v(s)
{
}

/*!
\brief Convert a series of displacements into positions.

Only the first vector will be kept as origin.
*/
VectorSet2 VectorSet2::VectorToPoint() const
{
  if (v.size() == 0)
    return VectorSet2();

  QVector<Vector2> p(v.size());

  p[0] = v[0];
  for (int i = 1; i < v.size(); i++)
  {
    p[i] = p[i - 1] + v[i];
  }
  return VectorSet2(p);
}

/*!
\brief Reverse the order of the elements.
*/
void VectorSet2::Reverse()
{
  const int n = v.size();
  for (int i = 0; i < n / 2; i++)
  {
    Swap(v[i], v[n - 1 - i]);
  }
}

/*!
\brief Compute the bounding box.

Simply calls Box::Box(const QVector&).
*/
Box2 VectorSet2::GetBox() const
{
  return Box2(v);
}

/*!
\brief Translate all the points.
\param t Translation vector.
*/
void VectorSet2::Translate(const Vector2& t)
{
  for (int i = 0; i < v.size(); i++)
  {
    v[i] += t;
  }
}

/*!
\brief Scale all the points.
\param s Scaling factor.
*/
void VectorSet2::Scale(const double& s)
{
  for (int i = 0; i < v.size(); i++)
  {
    v[i] *= s;
  }
}

/*!
\brief Scale all the points.
\param s Scaling vector.
*/
void VectorSet2::Scale(const Vector2& s)
{
  for (int i = 0; i < v.size(); i++)
  {
    v[i] *= s;
  }
}

//   // Initialize distance array to the distance to first point
//   // Distances will be stored in increasing order
//   QVector<double> r(n, Math::Infinity);

//   // Initialize distance array to the distance to first point
//   // Distances will be stored in increasing order
//   QVector<double> r;

//     // Here, k+1 is the index where the new index should inserted

//   // Initialize distance array to the distance to first point
//   // Distances will be stored in increasing order
//   QVector<double> r(n, Math::Infinity);

