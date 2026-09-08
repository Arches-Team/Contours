// Box
#pragma once

 #include <QtWidgets/QGraphicsScene>

#include "evector.h"

class Box
{
protected:
  Vector a = Vector(0.0), b = Vector(1.0); //!< Lower and upper vertexex.
public:
  //! Empty.
  Box() {}
  explicit Box(const double&);
  explicit Box(const Vector&, const Vector&);
  explicit Box(const Vector&, const double&);

  //! Empty.
  ~Box() {}

  // Access vertexes
  Vector& operator[] (int);
  Vector operator[] (int) const;

  // Acces to vertexes
  Vector Center() const;

  Vector Size() const;

};

//! Returns either end vertex of the box.
inline Vector& Box::operator[] (int i)
{
  if (i == 0) return a;
  else return b;
}

//! Overloaded.
inline Vector Box::operator[] (int i) const
{
  if (i == 0) return a;
  else return b;
}

//! Returns the center of the box.
inline Vector Box::Center() const
{
  return 0.5 * (a + b);
}

/*!
\brief Compute the size (width, length and height) of a box.
\sa Box::Diagonal()
*/
inline Vector Box::Size() const
{
  return b - a;
}

class Box2
{
protected:
  Vector2 a = Vector2(0.0), b = Vector2(1.0); //!< Lower and upper vertexes of the box.
public:
  Box2() {}
  explicit Box2(const double&);
  explicit Box2(const double&, const double&);
  explicit Box2(const Vector2&, const Vector2&);
  explicit Box2(const Vector2&, const double&);
  explicit Box2(const Vector2&, const double&, const double&);
  explicit Box2(const Box&);
  explicit Box2(const Box2&, const Box2&);
  explicit Box2(const QVector<Vector2>&);
  explicit Box2(const QSize&);

  Vector2& operator[] (int);
  Vector2 operator[] (int) const;

  double Width() const;
  double Height() const;
  Vector2 Diagonal() const;

  // Access to vertexes
  Vector2 Center() const;
  Vector2 Vertex(int) const;

  void Scale(const Vector2&);
  void Scale(const double&);

  Box2 Translated(const Vector2&) const;
  Box2 Scaled(const double&) const;
  Box2 ScaledCentered(const double&) const;

  Box2 Extended(const double&) const;

  // Inside
  bool Inside(const Vector2&) const;

  Box ToBox(const double&, const double&) const;

  void Draw(QGraphicsScene&, const QPen & = QPen(), const QBrush & = QBrush()) const;

  QRectF GetQtRect() const;

  Box2 Tile(int, int) const;

public:
  static const double epsilon;
  static const Box2 Null;
};

/*!
\brief Create a box.

Note that is possible to create a box using Vector as parameters
as the compiler will call the constructor Vector2::Vector2(const Vector&).

\param a,b Points.
*/
inline Box2::Box2(const Vector2& a, const Vector2& b) :a(a), b(b)
{
}

/*!
\brief Create a box in the plane given a box.
\param box The box.
*/
inline Box2::Box2(const Box& box) :a(box[0]), b(box[1])
{
}

/*!
\brief Creates a box.

\param c Center.
\param r Radius.
*/
inline Box2::Box2(const Vector2& c, const double& r)
{
  a = c - Vector2(r);
  b = c + Vector2(r);
}

/*!
\brief Create a box given a center point and its width, length, and height.

\param c center.
\param x,y Width and height.
*/
inline Box2::Box2(const Vector2& c, const double& x, const double& y)
{
  Vector2 r = 0.5 * Vector2(x, y);
  a = c - r;
  b = c + r;
}

/*!
\brief Create a square box centered at the origin and of given half side length.

This is equivalent to:
\code
Box2 box(Vector2(0.0),2.0);  // Simplified constructor Box2(2.0);
\endcode
\param r Half side length.
*/
inline Box2::Box2(const double& r)
{
  a = -Vector2(r);
  b = Vector2(r);
}

/*!
\brief Create a box centered at the origin and of given dimensions.

This is equivalent to:
\code
double width, height;
Box2 box(Vector2(-width/2.0,-height/2.0),Vector2(width/2.0,height/2.0);  // Simplified constructor Box2(width,height);
\endcode
\param x,y %Size.
*/
inline Box2::Box2(const double& x, const double& y)
{
  a = -Vector2(x / 2.0, y / 2.0);
  b = Vector2(x / 2.0, y / 2.0);
}

//! Returns either end vertex of the box.
inline Vector2& Box2::operator[] (int i)
{
  if (i == 0) return a;
  else return b;
}

//! Overloaded.
inline Vector2 Box2::operator[] (int i) const
{
  if (i == 0) return a;
  else return b;
}

/*!
\brief Compute the width of a box.

\sa Box2::Size()
*/
inline double Box2::Width() const
{
  return b[0] - a[0];
}

/*!
\brief Compute the height of a box.

\sa Box2::Size()
*/
inline double Box2::Height() const
{
  return b[1] - a[1];
}

/*!
\brief Returns the diagonal of the box.
*/
inline Vector2 Box2::Diagonal() const
{
  return (b - a);
}

/*!
\brief Returns the center of the box.
*/
inline Vector2 Box2::Center() const
{
  return 0.5 * (a + b);
}

/*!
\brief Returns the k-th vertex of the box.

The returned vector is computed by analysing the first two bits of k as follows:
\code
Vector2 vertex=Vector2((k&1)?b[0]:a[0],(k&2)?b[1]:a[1]);
\endcode
*/
inline Vector2 Box2::Vertex(int k) const
{
  return Vector2((k & 1) ? b[0] : a[0], (k & 2) ? b[1] : a[1]);
}

