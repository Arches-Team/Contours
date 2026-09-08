// Mesh

#include "mesh.h"

/*!
\class Mesh mesh.h

\brief Core triangle mesh class.
\ingroup ExtendedKernelGroup
*/

/*!
\brief Initialize the mesh to empty.
*/
Mesh::Mesh()
{
}

/*!
\brief Create a mesh from a set of vertices and a set of triangle indexes.

Indices must have a size multiple of three. Normals are computed to define flat triangles.

\param vertices List of vertices.
\param indices List of vertex indexes.
\param flat Boolean, set to true if face should be flat, set to false to generate smooth normals.
*/
Mesh::Mesh(const QVector<Vector>& vertices, const QVector<int>& indices, bool flat) :vertices(vertices), varray(indices)
{
  int nfaces = varray.size() / 3;
  narray.resize(indices.size());

  if (flat == true)
  {
    // Number of normals is equal to the number of faces
    normals.resize(nfaces);

    for (int i = 0; i < nfaces; i++)
    {
      Vector nt = GetTriangle(i).Normal();
      normals[i] = nt;
      narray[3 * i] = i;
      narray[3 * i + 1] = i;
      narray[3 * i + 2] = i;
    }
  }
  else
  {
    // Number of normals is equal to the number of vertexes
    normals.fill(Vector::Null, vertices.size());

    for (int i = 0; i < nfaces; i++)
    {
      Vector nt = GetTriangle(i).AreaNormal();
      normals[varray[3 * i]] += nt;
      normals[varray[3 * i + 1]] += nt;
      normals[varray[3 * i + 2]] += nt;
    }

    for (int i = 0; i < normals.size(); i++)
    {
      Normalize(normals[i]);
    }
    // The normal indexes are the same
    narray = indices;
  }
}

/*!
\brief Empty
*/
Mesh::~Mesh()
{
}

//   // Size: two half spheres and k circles defining the cylinder

//   // Apexes

// void Mesh::EdgeCollapse(int i)
// {
//   // std::cout << *this;
//   //std::cout << "Mesh::EdgeCollapse(int=" << i << " )" << std::endl;
//   //std::cout << "   which means i= " << i << " and next(i)= " << NextIndex(i) << std::endl;

//   //Math::Sort(ia, ib);

//   // std::cout << *this << std::endl;

//  // std::cout << "Base of i " << BaseIndex(i) << " and of k " << BaseIndex(k) << std::endl;

//   // Size: two half spheres and k circles defining the cylinder

//   // Size: two half spheres and k circles defining the cylinder

//   // Size: two half spheres and k circles defining the cylinder

//   // Size: two half spheres and k circles defining the cylinder

//     // Compute yz quadrangles 

//     // Compute xz quadrangles 
