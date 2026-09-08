// Segment
#pragma once

#include "evector.h"
#include <QtWidgets/QGraphicsScene>

// Segment class
class Segment2
{
protected:
  Vector2 a = Vector2::Null, b = Vector2::X; //!< End vertices of the segment.
public:
  //! Empty.
  Segment2() {}
  explicit Segment2(const Vector2&, const Vector2&);
  ~Segment2() {}

  // End vertices
  Vector2 Vertex(int) const;
  Vector2& Vertex(int);

  void DrawArrow(QGraphicsScene&, const double&, const QPen & = QPen(), const QBrush & = QBrush(QColor(255, 255, 255))) const;

  double R(const Vector2&) const;
  double R(const Vector2&, double&) const;

  bool Intersect(const Segment2&) const;

  static constexpr const double epsilon = 1e-8; //!< Epsilon value for intersection test
};

/*!
\brief Creates a planar segment given end vertices.
\param a, b End vertices of the segment.
*/
inline Segment2::Segment2(const Vector2& a, const Vector2& b) : a(a), b(b)
{
}

//! Return one of the end vertex of the axis.
inline Vector2 Segment2::Vertex(int i) const
{
  if (i == 0) return a;
  else return b;
}

//! Return one of the end vertex of the axis.
inline Vector2& Segment2::Vertex(int i)
{
  if (i == 0) return a;
  else return b;
}

class Polygons2;

class SegmentSet2
{
protected:
  QVector<Vector2> vertices; //!< Vertices.
  QVector<int> indices; //!< %Segment vertex indices.
public:
  explicit SegmentSet2();
  explicit SegmentSet2(const QVector<Vector2>&, const QVector<int>&);

  Segment2 GetSegment(int) const;

  int SegmentSize() const;
  Polygons2 GetPolygons(bool = false) const;

};

/*!
\brief Return the k-th segment.
\param k Index.
*/
inline Segment2 SegmentSet2::GetSegment(int k) const
{
  return Segment2(vertices.at(indices.at(2 * k)), vertices.at(indices.at(2 * k + 1)));
}

/*!
\brief Return the number of segments.
*/
inline int SegmentSet2::SegmentSize() const
{
  return indices.size() / 2;
}

