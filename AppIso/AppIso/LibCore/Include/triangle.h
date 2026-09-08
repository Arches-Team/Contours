// Triangle

#pragma once

#include "segment.h"
#include "circle.h"

// Triangle
class Triangle
{
protected:
  Vector p[3]; //!< %Array of vertices.
public:
  //! Empty.
  Triangle() {}
  explicit Triangle(const Vector&, const Vector&, const Vector&);

  //! Empty.
  ~Triangle() {}

  // Geometry
  Vector Normal() const;
  Vector AreaNormal() const;

};

/*!
\brief Create a triangle.
\param a,b,c Vertices of the triangle.
*/
inline Triangle::Triangle(const Vector& a, const Vector& b, const Vector& c) : p{ a,b,c }
{
}

class Triangle2
{
protected:
  Vector2 p[3]; //!< %Array of vertices.
public:
  //! Empty.
  Triangle2() {}
  explicit Triangle2(const Vector2&, const Vector2&, const Vector2&);

  //! Empty.
  ~Triangle2() {}

  Vector2 operator[] (int) const;
  Vector2& operator[] (int);

  // Geometry
  Vector2 Center() const;

  Circle2 Circumscribed() const;

  double Area() const;

  Vector BarycentricCoordinates(const Vector2&) const;

  bool Inside(const Vector2&) const;

  void Draw(QGraphicsScene&, const QPen & = QPen(), const QBrush & = QBrush()) const;

};

/*!
\brief Create a triangle given three points.
\param a, b, c Vertices.
*/
inline Triangle2::Triangle2(const Vector2& a, const Vector2& b, const Vector2& c) : p{ a,b,c }
{
}

/*!
\brief Return the i-th vertex.
*/
inline Vector2 Triangle2::operator[] (int i) const
{
  return p[i];
}

/*!
\brief Return a reference to the i-th vertex.
*/
inline Vector2& Triangle2::operator[](int i)
{
    return p[i];
}

/*!
\brief Compute the circle circumscribing the triangle.

This function directly relies on the constructor of the class.

\sa Circle2::Circle2(const Vector2&, const Vector2&, const Vector2&);
*/
inline Circle2 Triangle2::Circumscribed() const
{
  return Circle2(p[0], p[1], p[2]);
}

