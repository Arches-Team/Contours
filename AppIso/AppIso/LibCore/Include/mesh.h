#pragma once

#include "triangle.h"
#include "scalarfield.h"
#include "ia.h"
#include "box.h"

class Mesh
{
protected:
  QVector<Vector> vertices; //!< Vertices.
  QVector<Vector> normals;  //!< Normals.
  QVector<int> varray;      //!< Vertex indexes.
  QVector<int> narray;      //!< Normal indexes.
public:
  explicit Mesh();
  explicit Mesh(const QVector<Vector>&, const QVector<int>&, bool = true);
  ~Mesh();

  Triangle GetTriangle(int) const;

};

/*!
\brief Get a triangle.
\param i Index.
\return The triangle.
*/
inline Triangle Mesh::GetTriangle(int i) const
{
  return Triangle(vertices.at(varray.at(i * 3 + 0)), vertices.at(varray.at(i * 3 + 1)), vertices.at(varray.at(i * 3 + 2)));
}

class Mesh2
{
protected:
  QVector<Vector2> vertices; //!< Vertices.
  QVector<int> indices; //!< %Triangle vertex indices.
 public:
  explicit Mesh2();
  explicit Mesh2(const QVector<Vector2>&, const QVector<int>&);

  explicit Mesh2(const Box2&, int, int);

  ~Mesh2();

  Triangle2 GetTriangle(int) const;
  Vector2 Vertex(int) const;
  Vector2 Vertex(int, int) const;

  Box2 GetBox() const;

  int VertexSize() const;
  int TriangleSize() const;
  int IndexSize() const;

  int index(int) const;
  int index(int, int) const;

  void SetTrigonometric(bool = true);

  static Mesh2 Delaunay(const QVector<Vector2>&);

  Ia EdgeLengthRange() const;

  Mesh2 SubMesh(const QVector<int>&, const QSet<int> & = {}) const;
  Mesh2 SubMesh(const Box2&) const;
  Mesh2 SubMesh(const ScalarField2&, bool = false) const;

  Mesh Extrude(double, double) const;

public:
  static int NextIndex(int);

};

/*!
\brief Get a triangle.
\param i Index.
\return The triangle.
*/
inline Triangle2 Mesh2::GetTriangle(int i) const
{
  return Triangle2(vertices.at(indices.at(i * 3 + 0)), vertices.at(indices.at(i * 3 + 1)), vertices.at(indices.at(i * 3 + 2)));
}

/*!
\brief Get a vertex.
\param i The index of the wanted vertex.
\return The wanted vertex (as a 3D Vector).
*/
inline Vector2 Mesh2::Vertex(int i) const
{
  return vertices.at(i);
}

/*!
\brief Get a vertex from a specific triangle.
\param t The number of the triangle that contains the wanted vertex.
\param v The triangle vertex: 0, 1, or 2.
\return The wanted vertex.
*/
inline Vector2 Mesh2::Vertex(int t, int v) const
{
  return vertices.at(indices.at(t * 3 + v));
}

/*!
\brief Get the vertex index.
\param i The number in indices of the wanted vertex.
\return The index of the wanted vertex.
*/
inline int Mesh2::index(int i) const
{
  return indices.at(i);
}

/*!
\brief Get a vertex/normal index according to triangle information.
\param t The triangle number.
\param v The triangle vertex index.
\return The index of the wanted vertex.
*/
inline int Mesh2::index(int t, int v) const
{
  return indices.at(t * 3 + v);
}

/*!
\brief Get the number of triangles.

This is the size of the array of indexes divided by three.

\return The effective number of triangle in the geometry.
*/
inline int Mesh2::TriangleSize() const
{
  return indices.size() / 3;
}

/*!
\brief Get the size of the index array.

\sa Mesh2::TriangleSize()
*/
inline int Mesh2::IndexSize() const
{
  return indices.size();
}

/*!
\brief Get the number of vertices in the geometry.
\return The number of vertices in the geometry, in other words the size of vertices.
*/
inline int Mesh2::VertexSize() const
{
  return vertices.size();
}

/*!
\brief Compute the next index of an edge in a triangle.
\param i Index of the starting vertex of an edge.
*/
inline int Mesh2::NextIndex(int i)
{
  return ((i % 3) == 2) ? i - 2 : i + 1;
}
