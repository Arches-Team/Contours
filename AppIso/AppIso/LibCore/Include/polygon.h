#pragma once

#include <QtWidgets/QGraphicsScene>

#include "segment.h"
#include "triangle.h"
#include "box.h"

class Polygon2
{
protected:
  QVector<Vector2> q; //!< %Array of vertices.
public:
  Polygon2();
  explicit Polygon2(const Vector2&, const Vector2&, const Vector2&);
  explicit Polygon2(const QVector<Vector2>&);
  explicit Polygon2(const QVector<Vector2>&, const QVector<int>&);

  Box2 GetBox() const;

  Polygon2 Scaled(const Vector2&) const;
  Polygon2 Scaled(const double&) const;

  Polygon2 Translated(const Vector2&) const;

  void Scale(const double&);

  // Access to vertices
  Vector2& Vertex(int);
  Vector2 Vertex(int) const;

  QVector<Vector2> Vertices();
  const QVector<Vector2>& Vertices() const;

  Vector2 Edge(int) const;

  int Size() const;

  // Inside-outside
  bool Inside(const Vector2&) const;

  double Signed(const Vector2&) const;

  bool Intersect(const Triangle2&) const;
  bool Intersect(const Segment2&) const;

  // Area and perimeter
  double Area(bool = false) const;
  double Length() const;

  Vector2 Center() const;

  QPolygonF GetQt() const;

  void Draw(QGraphicsScene&, const QPen & = QPen(), const QBrush & = QBrush()) const;

protected:
  double RC(const Vector2&) const;
};

/*!
\brief Get the array of vertices.
*/
inline QVector<Vector2> Polygon2::Vertices()
{
  return q;
}

/*!
\brief Get the array of vertices.
*/
inline const QVector<Vector2>& Polygon2::Vertices() const
{
  return q;
}

/*!
\brief Create an empty polygon.
*/
inline Polygon2::Polygon2()
{
}

/*!
\brief Read write access to the i-th point.
\param i Index.
*/
inline Vector2& Polygon2::Vertex(int i)
{
  return q[i];
}

/*!
\brief Read only access to the i-th point.
\param i Index.
*/
inline Vector2 Polygon2::Vertex(int i) const
{
  return q.at(i);
}

/*!
\brief Return the i-th edge, starting from the i-th vertex.
\param i Index.
*/
inline Vector2 Polygon2::Edge(int i) const
{
  return q.at((i + 1) % q.size()) - q.at(i);
}

/*!
\brief Return the number of vertices of the polygon.
*/
inline int Polygon2::Size() const
{
  return q.size();
}

class Polygons2
{
protected:
  QVector<Polygon2> poly; //!< %Set of polygons.
public:
  Polygons2();

  int Size() const;
  //! Empty
  ~Polygons2() {}

  const Polygon2& At(int) const;

  void Add(const Polygon2&);
};

/*!
\brief Return the size of the set.
*/
inline int Polygons2::Size() const
{
  return poly.size();
}

/*!
\brief Access.
\param i Index.
*/
inline const Polygon2& Polygons2::At(int i) const
{
  return poly.at(i);
}