// Mesh

#include "mesh.h"

//   // Close grid

//   // Parcours de l'ensemble des points de la courbe

//   // Calcul des normals

//   // Definition des indices des quads

//   // Contour d'init debut

/*!
\brief Extrude a mesh into a prism.
\param mesh The mesh.
\param a,b Extrusion elevations.
\author Benoit Huftier
*/
Mesh Mesh2::Extrude(double a, double b) const
{
  QVector<Vector> vertices;
  vertices.reserve(VertexSize() * 2);

  // Up vertices
  for (int i = 0; i < VertexSize(); i++)
    vertices.append(Vertex(i).ToVector(b));

  // Down vertices
  for (int i = 0; i < VertexSize(); i++)
    vertices.append(Vertex(i).ToVector(a));

  // Check edges
  QSet<int> edges;
  auto GetEdgeInd = [&](int a, int b) { return Math::Min(a, b) * VertexSize() + Math::Max(a, b); };

  for (int i = 0; i < TriangleSize(); i++)
  {
    for (int j = 0; j < 3; j++)
    {
      int e = GetEdgeInd(index(i, j), index(i, (j + 1) % 3));

      if (edges.contains(e)) edges.remove(e);
      else edges.insert(e);
    }
  }

  QVector<int> indices;
  indices.reserve(IndexSize() * 2 + edges.size() * 2);

  for (int i = 0; i < TriangleSize(); i++)
  {
    // Up triangle
    indices.append(index(i, 0));
    indices.append(index(i, 1));
    indices.append(index(i, 2));

    // Down triangle
    indices.append(index(i, 2) + VertexSize());
    indices.append(index(i, 1) + VertexSize());
    indices.append(index(i, 0) + VertexSize());

    // Edges triangles
    for (int j = 0; j < 3; j++)
    {
      int e = GetEdgeInd(index(i, j), index(i, (j + 1) % 3));

      if (edges.contains(e))
      {
        indices.append(index(i, j));
        indices.append(index(i, j) + VertexSize());
        indices.append(index(i, (j + 1) % 3));

        indices.append(index(i, (j + 1) % 3));
        indices.append(index(i, j) + VertexSize());
        indices.append(index(i, (j + 1) % 3) + VertexSize());
      }
    }
  }

  return Mesh(vertices, indices);
}