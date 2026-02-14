#pragma once

// FaceOp structure for a Mesh with an infinite point
// Each vertex points to a face and each face points to its 3 neighbouring faces
// Note that the Mesh should have no articulation point (TODO: check how to deal with them in the constructor)
// Also note that if the initial mesh has holes, the mesh + the infinite point is not manifold and the list of neighbours of the infinite point will be corrupted
// Both use cases should be rare and are thus not considered, but you should be aware of these problems
class Tin2 : public Mesh2
{
protected:
    QVector<int> triangles;  //!< each vertex has a reference to one adjacent triangle
    QVector<int> neighbours; //!< each triangle has a reference to its 3 neighbouring faces

    QVector<int> infiniteIndices; //!< like Mesh2::indices but for infinite triangles (to avoid corruption of Mesh2::TriangleSize with infinite triangles)
    
    QSet<int> articulationPoints; //!< points we should take care about because problems could occur.

public:
    Tin2(const Mesh2&);

    int TriangleFacingVertex(int, int) const;
    bool IsBorderTriangle(int) const;
    bool IsBorderVertex(int) const;
    bool IsAloneVertex(int) const;
    bool IsInfiniteTriangle(int) const;
    bool IsInfinitePoint(int) const;

    QVector<int> VertexNeighboursTriangles(int) const;
    QVector<int> VertexNeighboursVertices(int) const;

    // Debug
    QGraphicsScene* DebugScene(bool = false, bool = false, bool = false, bool = false, bool = false) const;

protected:
    QVector<int> VertexNeighboursTriangles(int, bool) const;

    int InfinitePoint() const;
    int FirstInfiniteTriangle() const;
    int InfiniteTriangleSize() const;
    
    int LocalTriangleId(int, int) const;
    int NextTriangleAroundVertex(int, int) const;
    int PreviousTriangleAroundVertex(int, int) const;
};