// Mesh

#include "mesh.h"
#include "polygon.h"
#include "circle.h"

/*!
\brief Create an empty mesh.
*/
Mesh2::Mesh2()
{
}

/*!
\brief Create a mesh from a list of vertices and a indexes.

Indices should be a multiple of three; the number of triangles is derived
from the size of indices.

\sa Mesh2::TriangleSize

\param v Set of vertices.
\param i Indexes that represent the triangles.
*/
Mesh2::Mesh2(const QVector<Vector2>& v, const QVector<int>& i) :vertices(v), indices(i)
{
}

/*!
\brief Empty
*/
Mesh2::~Mesh2()
{
}

/*!
\brief Generate a grid geometry.
\param box The box.
\param x, y Number of points in the subdivision.
*/
Mesh2::Mesh2(const Box2& box, int x, int y)
{
  Array2 a(box, x, y);

  vertices.resize(x * y);
  indices.resize(3 * (x - 1) * (y - 1) * 2);

  // Vertices
  for (int j = 0; j < x; j++)
  {
    for (int i = 0; i < y; i++)
    {
      vertices[j * x + i] = a.ArrayVertex(i, j);
    }
  }

  // Triangles
  int n = 0;
  for (int j = 0; j < x - 1; j++)
  {
    for (int i = 0; i < y - 1; i++)
    {
      indices[n + 0] = j * x + i;
      indices[n + 1] = j * x + i + 1;
      indices[n + 2] = (j + 1) * x + i;
      n += 3;
      indices[n + 0] = j * x + i + 1;
      indices[n + 1] = (j + 1) * x + i + 1;
      indices[n + 2] = (j + 1) * x + i;
      n += 3;
    }
  }
}

/*!
\brief Compute the Delaunay triangulation of a set of points.

In some cases, the triangulation may contain anti-trigonometric order triangles.
Therefore the function calls  SetTrigonometric(true) to guarantee that all triangles are properly oriented.

\param p Point set.

Bowyer-Watson algorithm, O(n log n) complexity, C++ implementation of http://paulbourke.net/papers/triangulate.
*/
Mesh2 Mesh2::Delaunay(const QVector<Vector2>& p)
{
  if (p.size() < 3)
    return Mesh2();

  double eps = 1e-4;
  std::vector<int> ltr; // triangles = 3 indexes

  // Init Data
  // Bounding box
  Box2 box(p);

  const double d = NormInfinity(box.Diagonal());
  const Vector2 c = box.Center();

  // Initialize
  Vector2 p0 = c + Vector2(-20.0 * d, -d);
  Vector2 p1 = c + Vector2(0.0, 20 * d);
  Vector2 p2 = c + Vector2(20 * d, -d);

  QVector<Vector2> points = p;

  points.append(p0); ltr.emplace_back(points.size() - 1);
  points.append(p1); ltr.emplace_back(points.size() - 1);
  points.append(p2); ltr.emplace_back(points.size() - 1);

  // Triangulation
  for (int i = 0; i < points.size() - 3; i++)
  {
    std::vector<QPair<int, int>> edges;
    std::vector<int> tmps;

    for (int j = 0; j < ltr.size(); j += 3)
    {
      // Check if the point is inside the triangle circumcircle.
      Circle2 circle = Triangle2(points[ltr[j + 0]], points[ltr[j + 1]], points[ltr[j + 2]]).Circumscribed();
      const double dist = SquaredNorm(circle.Center() - points[i]);
      if ((dist - circle.Radius() * circle.Radius()) <= eps)
      {
        edges.push_back(qMakePair(ltr[j + 0], ltr[j + 1]));
        edges.push_back(qMakePair(ltr[j + 0], ltr[j + 2]));
        edges.push_back(qMakePair(ltr[j + 1], ltr[j + 2]));
      }
      else {
        tmps.push_back(ltr[j + 0]);
        tmps.push_back(ltr[j + 1]);
        tmps.push_back(ltr[j + 2]);
      }
    }

    // Delete duplicate edges. 
    std::vector<bool> remove(edges.size(), false);
    for (auto it1 = edges.begin(); it1 != edges.end(); ++it1)
    {
      for (auto it2 = edges.begin(); it2 != edges.end(); ++it2)
      {
        if (it1 == it2) {
          continue;
        }
        if (*it1 == *it2) {
          remove[std::distance(edges.begin(), it1)] = true;
          remove[std::distance(edges.begin(), it2)] = true;
        }
      }
    }

    edges.erase(
      std::remove_if(edges.begin(), edges.end(),
        [&](auto const& e) { return remove[&e - &edges[0]]; }),
      edges.end());

    // Update triangulation.
    for (int j = 0; j < edges.size(); j++)
    {
      tmps.push_back(edges[j].first);
      tmps.push_back(edges[j].second);
      tmps.push_back(i);
    }
    ltr = tmps;
  }

  // Remove original super triangle.
  points.removeLast(); points.removeLast(); points.removeLast();
  for (int j = 0; j < ltr.size(); j += 3) {

    if ((ltr[j + 0] > points.size() - 1) || (ltr[j + 1] > points.size() - 1) || (ltr[j + 2] > points.size() - 1))
    {
      ltr.erase(ltr.begin() + j, ltr.begin() + j + 3);
      j -= 3;
    }
  }

  // Define mesh
  Mesh2 mesh(points, QVector<int>(ltr.begin(), ltr.end()));
  mesh.SetTrigonometric(true);
  return mesh;
}

/*!
\brief Compute the boudning box.

Simply eqsuivalent as:
\code
Mesh2 mesh;
Box2 box=Box2(mesh.Vertices());
\endcode
*/
Box2 Mesh2::GetBox() const
{
  return Box2(vertices);
}

/*!
\brief Set all triangles in the same trigonometric order.
\param trigo Trigonometric (counterclockwise) is true, clockwise if false.
\author Hugo Schott
*/
void Mesh2::SetTrigonometric(bool trigo)
{
  int order = trigo ? 1 : -1;

  const int nt = TriangleSize();

  for (int i = 0; i < nt; i++)
  {
    const Triangle2 t = GetTriangle(i);
    Vector2 e = t[1] - t[0];
    Vector2 n = e.Orthogonal();
    if (order * n * (t[2] - t[0]) < 0.0)
    {
      Math::Swap(indices[3 * i], indices[3 * i + 1]);
    }
  }
}

/*!
\brief Compute the range of edge lengths.
*/
Ia Mesh2::EdgeLengthRange() const
{
  if (indices.size() == 0)
    return Ia();

  Ia ab = Ia(Norm(vertices[indices[1]] - vertices[indices[0]]));

  // For all edges of the triangle
  for (int i = 0; i < indices.size(); i += 3)
  {
    const Vector2& va = vertices[indices[i + 0]];
    const Vector2& vb = vertices[indices[i + 1]];
    const Vector2& vc = vertices[indices[i + 2]];

    // Edge lengths
    // Edge lengths
    Math::SetMinMax(Norm(vb - va), ab[0], ab[1]);
    Math::SetMinMax(Norm(vc - vb), ab[0], ab[1]);
    Math::SetMinMax(Norm(va - vc), ab[0], ab[1]);
  }
  return ab;
}

//   //Math::Sort(ia, ib);

//   // std::cout << *this << std::endl;

//   //std::cout << "Base of i " << BaseIndex(i) << " and of k " << BaseIndex(k) << std::endl;

/*!
\brief Create a mesh by removing vertices that are not in the list (and the triangles connected to these vertices).
\param verticesInd Indexes of the preserved vertexes.
\param removedTrianglesInd Extra triangles to remove within the submesh.
\author Benoit Huftier
*/
Mesh2 Mesh2::SubMesh(const QVector<int>& verticesInd, const QSet<int>& removedTrianglesInd) const
{
  // If -1, it means that the vertex is not in the new mesh otherwise the value is the index of the vertex in the new mesh 
  QVector<int> indicesMap(vertices.size(), -1);
  QVector<Vector2> points; // points of the new mesh
  QVector<int> indices;    // triangles of the new mesh

  // Add points + create map indices
  for (int vi = 0; vi < verticesInd.size(); vi++)
  {
    indicesMap[verticesInd[vi]] = vi;
    points.append(vertices[verticesInd[vi]]);
  }

  // Add triangles
  for (int ti = 0; ti < TriangleSize(); ti++)
  {
    int vi1 = index(ti, 0);
    int vi2 = index(ti, 1);
    int vi3 = index(ti, 2);

    if (indicesMap[vi1] == -1)
      continue;
    if (indicesMap[vi2] == -1)
      continue;
    if (indicesMap[vi3] == -1)
      continue;
    if (removedTrianglesInd.contains(ti))
      continue;

    indices.append(indicesMap[vi1]);
    indices.append(indicesMap[vi2]);
    indices.append(indicesMap[vi3]);
  }

  return Mesh2(points, indices);
}

/*!
\brief Select the sub-mesh by removing all vertices outside the given box.
\param box The box.
\author Benoit Huftier
*/
Mesh2 Mesh2::SubMesh(const Box2& box) const
{
  QVector<int> keep;
  for (int vi = 0; vi < vertices.size(); vi++)
  {
    Vector2 p = Vertex(vi);
    if (box.Inside(p))
    {
      keep.append(vi);
    }
  }
  return SubMesh(keep);
}

/*!
\brief Select the sub-mesh by removing all vertices outside the given mask.
\param mask                     The mask. <= 0 means not taken, > 0 means inside the mask
\param removeCrossingTriangles  Should we remove triangles where three points are inside the mask but a part of the triangle is outside?
\author Benoit Huftier
*/
Mesh2 Mesh2::SubMesh(const ScalarField2& mask, bool removeTriangles) const
{
  QVector<int> keep;
  for (int vi = 0; vi < vertices.size(); vi++)
  {
    Vector2 p = Vertex(vi);
    if (mask.Value(p) > 0)
    {
      keep.append(vi);
    }
  }

  if (!removeTriangles)
    return SubMesh(keep);

  // TODO: we need to do that because we cannot have intersections between a mask and triangles
  Polygons2 polys = mask.LineSegments(0.0000001, true).GetPolygons();

  QSet<int> removedTriangles;
  for (int ti = 0; ti < TriangleSize(); ti++)
  {
    int i0 = index(ti, 0);
    int i1 = index(ti, 1);
    int i2 = index(ti, 2);
    if (keep.contains(i0) && keep.contains(i1) && keep.contains(i2))
    {
      Triangle2 t = GetTriangle(ti);
      for (int pi = 0; pi < polys.Size(); ++pi)
      {
        if (polys.At(pi).Intersect(t))
        {
          removedTriangles.insert(ti);
          break;
        }
      }
    }
  }

  return SubMesh(keep, removedTriangles);
}

