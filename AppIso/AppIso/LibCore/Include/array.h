// Fundamentals
#pragma once

#include "box.h"

class Array2 : protected Box2
{
public:
  int nx = 0, ny = 0; //!< Sizes.
  Vector2 celldiagonal = Vector2::Null; //!< Cell diagonal.
  Vector2 inversecelldiagonal = Vector2::Null; //!< Inverse cell diagonal.
public:
  Array2();
  explicit Array2(const Box2&, int, int);

  int VertexSize() const;
  int GetSizeX() const;
  int GetSizeY() const;

  int VertexBorderSize() const;

  Vector2 CellCenter(int, int) const;

  Box2 GetBox() const;

  Vector2 CellDiagonal() const;

  Vector2 ArrayVertex(int, int) const;

  QPoint VertexBorder(int) const;
  int VertexBorderIndex(int, int) const;

  void CellInteger(const Vector2&, int&, int&, double&, double&) const;

//   // Domain queries
  constexpr bool InsideVertexIndex(int, int) const;
  constexpr bool InsideVertexIndex(const QPoint&) const;

  // Indexes for storing elements at vertices
  constexpr int VertexIndex(int, int) const;

  QPoint Next(const QPoint&, int) const;

  constexpr bool InsideCellIndex(int, int) const;
protected:
  static const QPoint next[8]; //!< %Array of points in the 1-ring neighborhood.
  static const double inverselength[8]; //!< Inverse length.
};

/*!
\brief Get the vertex size of the array for x axis.
*/
inline int Array2::GetSizeX() const
{
  return nx;
}

/*!
\brief Get the vertex size of the array for y axis.
*/
inline int Array2::GetSizeY() const
{
  return ny;
}

/*!
\brief Return the size of the vertex array.
*/
inline int Array2::VertexSize() const
{
  return nx * ny;
}

/*!
\brief Return the number of vertices on the boundary of the rectangle.
*/
inline int Array2::VertexBorderSize() const
{
  return nx * 2 + ny * 2 - 4;
}

/*!
\brief Compute the coordinates of a point on the lattice.
\param i,j Integer coordinates.
*/
inline Vector2 Array2::ArrayVertex(int i, int j) const
{
  return Vector2(a[0] + i * celldiagonal[0], a[1] + j * celldiagonal[1]);
}

/*!
\brief Get the box of the array.
*/
inline Box2 Array2::GetBox() const
{
  return Box2(a, b);
}

/*!
\brief Compute the index of a given cell.
\param i,j Integer coordinates of the cell.
*/
inline constexpr int Array2::VertexIndex(int i, int j) const
{
  return i + nx * j;
}

/*!
\brief Check if the indexes are within range.
\param i,j Integer coordinates of the vertex.
*/
inline constexpr bool Array2::InsideCellIndex(int i, int j) const
{
  return (i >= 0) && (i < nx - 1) && (j >= 0) && (j < ny - 1);
}

/*!
\brief Check if the indexes are within range.
\param i,j Integer coordinates of the vertex.
*/
inline constexpr bool Array2::InsideVertexIndex(int i, int j) const
{
  return (i >= 0) && (i < nx) && (j >= 0) && (j < ny);
}

/*!
\brief Check if the indexes are within range.
\param p Point.
*/
inline constexpr bool Array2::InsideVertexIndex(const QPoint& p) const
{
  return (p.x() >= 0) && (p.x() < nx) && (p.y() >= 0) && (p.y() < ny);
}

/*!
\brief Compute the point on the lattice given an input point.
\param p Point.
\param i,j Integer coordinates of the cell.
\param u,v Coordinates of the point in the corresponding cell.
*/
inline void Array2::CellInteger(const Vector2& p, int& i, int& j, double& u, double& v) const
{
  Vector2 q = p - a;

  /*
    Vector2 d = b - a;

    u = q[0] / d[0];
    v = q[1] / d[1];

    // Scale
    u *= (nx - 1);
    v *= (ny - 1);
  */
  u = q[0] * inversecelldiagonal[0];
  v = q[1] * inversecelldiagonal[1];

  // Integer coordinates
  i = int(u);
  j = int(v);

  // Local coordinates within cell
  u -= i;
  v -= j;
}

/*!
\brief Compute the point next to another one.
\param p Point.
\param n Next neighbor, should be in [0,7].
*/
inline QPoint Array2::Next(const QPoint& p, int n) const
{
  return p + next[n];
}

