#include "tin.h"
#include "sampling.h"
#include "draw.h"

#include "cpu.h"

Tin2::Tin2(const Mesh2& mesh) : Mesh2(mesh)
{
    // -1 if a vertex is not connected to any triangle
    triangles = QVector<int>(VertexSize(), -1);

    // Every triangle needs to point towards the sky
    SetTrigonometric(true);

    // Structure to remember which triangle is next to which edge
    // The id of an edge "ab" is a + n * b where n is the number of vertices
    // Note that n*n should not be more than the size of a long long
    using edge_ind_t = long long;
    QMap<edge_ind_t, int> edges;
    int n = VertexSize() + 1; // +1 considering the infinite vertex

    constexpr edge_ind_t lim1 = std::numeric_limits<edge_ind_t>::max();
    constexpr edge_ind_t lim2 = std::numeric_limits<int>::max();
    edge_ind_t maxn1 = Math::Floor(sqrt(lim1));
    int maxn2 = lim2 / 3;
    if (n > Math::Min(maxn1, maxn2))
    {
        qDebug() << "You ask to create a TIN with" << n << "nodes, but the maximum allow TIN node number is min(" << maxn1 << "," << maxn2 << "). The first is due to the way the edges are stored. The second is due to the number of indices max we can have with an int.";
    }

    for (int ti = 0; ti < TriangleSize(); ++ti)
    {
        // The 3 vertices of the triangle
        edge_ind_t i1 = indices[3 * ti];
        edge_ind_t i2 = indices[3 * ti + 1];
        edge_ind_t i3 = indices[3 * ti + 2];

        // We update the triangle each vertex is pointing at
        triangles[i1] = ti;
        triangles[i2] = ti;
        triangles[i3] = ti;

        // Id of the three edges
        edge_ind_t e12 = i1 + i2 * n;
        edge_ind_t e23 = i2 + i3 * n;
        edge_ind_t e31 = i3 + i1 * n;

        // Updating the edges
        edges[e12] = ti;
        edges[e23] = ti;
        edges[e31] = ti;
    }

    // The triangle pointing by the infinite point
    triangles.append(FirstInfiniteTriangle());
    
    articulationPoints.clear();
    int new_ti = FirstInfiniteTriangle();
    // Now we can use our structure to create infinite triangles where an edge is missing
    for (int ti = 0; ti < TriangleSize(); ++ti)
    {
        // The 3 vertices of the triangle
        edge_ind_t i1 = indices[3 * ti];
        edge_ind_t i2 = indices[3 * ti + 1];
        edge_ind_t i3 = indices[3 * ti + 2];

        // Id of the reversed three edges
        edge_ind_t e21 = i2 + i1 * n;
        edge_ind_t e32 = i3 + i2 * n;
        edge_ind_t e13 = i1 + i3 * n;

        // infinite point ind
        edge_ind_t infp = InfinitePoint();
    
        // If the reverse id does not exist, it means that the edge is on the border of the Mesh so we create an infinite triangle
        if (!edges.contains(e32))
        {
            infiniteIndices.append(i3);
            infiniteIndices.append(i2);
            infiniteIndices.append(infp);

            edge_ind_t e2i = i2 + infp * n;
            edge_ind_t ei3 = infp + i3 * n;
            if (edges.contains(e2i) || edges.contains(ei3))
                articulationPoints.insert(i1);
            edges[e32] = new_ti;
            edges[e2i] = new_ti;
            edges[ei3] = new_ti;
            new_ti++;
        }
        if (!edges.contains(e13))
        {
            infiniteIndices.append(i1);
            infiniteIndices.append(i3);
            infiniteIndices.append(infp);

            edge_ind_t e3i = i3 + infp * n;
            edge_ind_t ei1 = infp + i1 * n;
            if (edges.contains(e3i) || edges.contains(ei1))
                articulationPoints.insert(i2);
            edges[e13] = new_ti;
            edges[e3i] = new_ti;
            edges[ei1] = new_ti;
            new_ti++;
        }
        if (!edges.contains(e21))
        {
            infiniteIndices.append(i2);
            infiniteIndices.append(i1);
            infiniteIndices.append(infp);

            edge_ind_t e1i = i1 + infp * n;
            edge_ind_t ei2 = infp + i2 * n;
            if (edges.contains(e1i) || edges.contains(ei2))
                articulationPoints.insert(i3);
            edges[e21] = new_ti;
            edges[e1i] = new_ti;
            edges[ei2] = new_ti;
            new_ti++;
        }
    }

    if (!articulationPoints.empty())
    {
        // TODO: The mesh cannot have articulation point because if vi were an articulation point, there will be multiple edges "vi -- infinite point" and they'll have the same id
        // Exemple of an articulation point O:
        // 
        //     A *-------* B
        //        \     /
        //         \   /
        //          \ /
        // C *-------*-------* E
        //    \     /O\     / 
        //     \   /   \   /  
        //      \ /     \ /   
        //       *       *
        //       D       F
        // 
        // Idea to handle it : keep articulation point, then for each of them, loop over all triangles to find neighbours triangles (OAB, OCD, OFE in the example). If the Mesh is planar with no intersecting triangles, we should be able to get all vectors (OE, OB, OA, OC, OD, OF in the example) and sort them in trigonometric order to find which edges are neighbours. Then we find a way to give ids to them, like id + t1 * n*n + t2 * n*n*m where t1 and t2 are the triangles and m is the number of triangles. Then we should check in the reconstruction below if the edge have an articulation point and if yes apply the reversed id + t2 * n*n + t1 * n*n*m.
        QDebug dbg = qDebug().nospace();
        dbg << "Articulation point detected. Undefined behaviour\n";
        for (int i : articulationPoints)
        {
            dbg << i << ", ";
        }
        dbg << "";
    }

    neighbours = QVector<int>((TriangleSize() + InfiniteTriangleSize()) * 3, -1);

    // Now we are sure that every edges have two adjacent triangles, so we can create neighbourhood
    for (int ti = 0; ti < TriangleSize() + InfiniteTriangleSize(); ++ti)
    {
        // The 3 vertices of the triangle
        edge_ind_t i1 = ti >= TriangleSize() ? infiniteIndices[3 * (ti - FirstInfiniteTriangle())] : indices[3 * ti];
        edge_ind_t i2 = ti >= TriangleSize() ? infiniteIndices[3 * (ti - FirstInfiniteTriangle()) + 1] : indices[3 * ti + 1];
        edge_ind_t i3 = ti >= TriangleSize() ? infiniteIndices[3 * (ti - FirstInfiniteTriangle()) + 2] : indices[3 * ti + 2];

        // Id of the reversed three edges
        edge_ind_t e21 = i2 + i1 * n;
        edge_ind_t e32 = i3 + i2 * n;
        edge_ind_t e13 = i1 + i3 * n;

        if (!edges.contains(e21))
            qDebug() << "Error: a triangle as no neighbour on one edge: this should not happend";
        if (!edges.contains(e32))
            qDebug() << "Error: a triangle as no neighbour on one edge: this should not happend";
        if (!edges.contains(e13))
            qDebug() << "Error: a triangle as no neighbour on one edge: this should not happend";

        neighbours[3 * ti] = edges[e32];
        neighbours[3 * ti + 1] = edges[e13];
        neighbours[3 * ti + 2] = edges[e21];
    }
}

/*!
\brief Get a triangle index according to triangle information.
\param t The triangle number.
\param v The triangle vertex index.
\return The index of the wanted triangle. /!\ the return value can be an infinite triangle!
*/
int Tin2::TriangleFacingVertex(int t, int v) const
{
    return neighbours[3 * t + v];
}

/*!
\brief Checks if a triangle is on the border of the mesh (one neighbour is missing)
*/
bool Tin2::IsBorderTriangle(int ti) const
{
    return IsInfiniteTriangle(TriangleFacingVertex(ti, 0))
        || IsInfiniteTriangle(TriangleFacingVertex(ti, 1))
        || IsInfiniteTriangle(TriangleFacingVertex(ti, 2));
}

/*!
\brief Checks if a vertex is on the hull of the mesh
*/
bool Tin2::IsBorderVertex(int vi) const
{
    if (IsAloneVertex(vi))
    {
        return true;
    }

    for (int ti : VertexNeighboursTriangles(vi, true))
    {
        if (IsInfiniteTriangle(ti))
        {
            return true;
        }
    }
    return false;
}

/*!
\brief Checks if a vertex has no neighbours
*/
bool Tin2::IsAloneVertex(int vi) const
{
    return triangles[vi] == -1;
}

/*!
\brief Return if ti is an infinite triangle
*/
bool Tin2::IsInfiniteTriangle(int ti) const
{
    return ti >= FirstInfiniteTriangle();
}

/*!
\brief Return if vi is the infinite point
*/
bool Tin2::IsInfinitePoint(int vi) const
{
    return vi == InfinitePoint();
}

/*!
\brief List of triangles neighbouring vertex i in counterclockwise
*/
QVector<int> Tin2::VertexNeighboursTriangles(int vi) const
{
    return VertexNeighboursTriangles(vi, false);
}

/*!
\brief List of triangles neighbouring vertex i in counterclockwise
\param vi                   index of the vertex
\param withInfiniteTriangle if the return array contains infinite triangla

This method is protected because we don't want the user to be able to access infinite triangles
*/
QVector<int> Tin2::VertexNeighboursTriangles(int vi, bool withInfiniteTriangle) const
{
    // Nothing prevent a vertex to have no neighbour triangle, in this case, ti == -1 (see constructor for more details)
    if (IsAloneVertex(vi))
        return {};

    int ti = triangles[vi];
    QVector<int> vNeighbours;

    int loopcount = 0;
    do
    {
        if (withInfiniteTriangle || !IsInfiniteTriangle(ti))
            vNeighbours.append(ti);
        ti = NextTriangleAroundVertex(vi, ti);
        // TODO: should be removed once the problem with articulation points is resolved
        // We can loop forever sometimes because of bad neighbouring
        if (loopcount++ > 50)
            break;
    }
    // While the turn is not over, we turn again
    while (ti != triangles[vi]);

    return vNeighbours;
}

/*!
\brief List of vertices neighbouring vertex i
*/
QVector<int> Tin2::VertexNeighboursVertices(int vi) const
{
    QVector<int> vNeighbours;

    // We add all points of the neighbouring triangle.
    // Two neighbour triangles are often adjacent (but not always) so we take care to not write twice the same point
    for (int ti : VertexNeighboursTriangles(vi))
    {
        int loc_vi = LocalTriangleId(ti, vi);
        int neigh_i1 = index(ti, (loc_vi + 1) % 3);
        int neigh_i2 = index(ti, (loc_vi + 2) % 3);
        if (vNeighbours.empty() || vNeighbours.last() != neigh_i1)
            vNeighbours.append(neigh_i1);
        vNeighbours.append(neigh_i2);
    }

    // Same, we take care not adding doublons
    if (!vNeighbours.empty() && vNeighbours.first() == vNeighbours.last())
        vNeighbours.removeLast();

    return vNeighbours;
}

/*!
\brief Return the indices corresponding to the infinite point
*/
int Tin2::InfinitePoint() const
{
    return VertexSize();
}

/*!
\brief Indices of the first infinite triangles (for the neighbours array)
*/
int Tin2::FirstInfiniteTriangle() const
{
    return TriangleSize();
}

/*!
\brief Number of infinite triangles
*/
int Tin2::InfiniteTriangleSize() const
{
    return infiniteIndices.size() / 3;
}

/*!
\brief Return 0/1/2 according to vertex position in the triangle vertex ordering 
*/
int Tin2::LocalTriangleId(int ti, int vi) const
{
    if (IsInfiniteTriangle(ti))
    {
        if (vi == infiniteIndices[3 * (ti - FirstInfiniteTriangle())])
            return 0;
        if (vi == infiniteIndices[3 * (ti - FirstInfiniteTriangle()) + 1])
            return 1;
        if (vi == infiniteIndices[3 * (ti - FirstInfiniteTriangle()) + 2])
            return 2;
    }
    else
    {
        if (vi == indices[3 * ti])
            return 0;
        if (vi == indices[3 * ti + 1])
            return 1;
        if (vi == indices[3 * ti + 2])
            return 2;
    }

    qDebug() << vi << "is not a vertex of the triangle" << ti;
    exit(1);
}

/*!
\brief Return the next counterclockwise triangle id around vertex vi
\param vi vertex id
\param ti current triangle id               (/!\ Should be a triangle around vi)
\return next triangle id counterclockwise   (/!\ Can be -1)
*/
int Tin2::NextTriangleAroundVertex(int vi, int ti) const
{
    int loc_vi = LocalTriangleId(ti, vi);
    int next = TriangleFacingVertex(ti, (loc_vi + 1) % 3);

    return next;
}

/*!
\brief Return the previous counterclockwise triangle id around vertex vi
\param vi vertex id
\param ti current triangle id                   (/!\ Should be a triangle around vi)
\return previous triangle id counterclockwise   (/!\ Can be -1)
*/
int Tin2::PreviousTriangleAroundVertex(int vi, int ti) const
{
    int loc_vi = LocalTriangleId(ti, vi);
    int prev = TriangleFacingVertex(ti, (loc_vi + 2) % 3);

    return prev;
}

/*!
\brief Debug scene
\param a bunch of bools to display more or less
*/
QGraphicsScene* Tin2::DebugScene(bool borders, bool trianglesNeighbours, bool verticesTriangle, bool verticesNeighboursTriangles, bool verticesNeighboursVertices) const
{
    QGraphicsScene* scene = new QGraphicsScene;
    if (VertexSize() == 0)
        return scene;

    Ia range = EdgeLengthRange();
    double r = range[0] / 4.;
    Box2 b = GetBox();
    Vector2 border = 2 * Vector2(r, r);

    Box2(b[0] - border, b[1] + border).Draw(*scene, QPen(Qt::white), QBrush(Qt::white));

    // Triangles
    for (int ti = 0; ti < TriangleSize(); ++ti)
    {
        if (borders && IsBorderTriangle(ti))
            GetTriangle(ti).Draw(*scene, QPen(QBrush(Qt::black), r / 8), QBrush(Qt::cyan));
        else
            GetTriangle(ti).Draw(*scene, QPen(QBrush(Qt::black), r / 8));
    }

    // Neighbouring triangles of triangles
    if (trianglesNeighbours)
    {
        for (int ti = 0; ti < TriangleSize(); ++ti)
        {
            Vector2 b1 = GetTriangle(ti).Center();
            for (int ni = 0; ni < 3; ++ni)
            {
                int tni = TriangleFacingVertex(ti, ni);
                if (!IsInfiniteTriangle(tni))
                {
                    Vector2 b2 = GetTriangle(tni).Center();
                    Vector2 dir = Normalized(b2 - b1);
                    Vector2 orth = dir.Orthogonal();

                    Segment2 s(b1 + (r / 4) * orth + (r / 3) * dir, b2 + (r / 4) * orth - (r / 3) * dir);
                    s.DrawArrow(*scene, r / 8, QPen(QBrush(Qt::darkMagenta), r / 8));
                }
            }
        }
    }

    // Refering triangle of vertices
    if (verticesTriangle)
    {
        for (int vi = 0; vi < VertexSize(); ++vi)
        {
            if (!IsAloneVertex(vi))
            {
                Vector2 p1 = vertices[vi];
                Vector2 c = GetTriangle(triangles[vi]).Center();
                Vector2 dir = c - p1;
                Segment2 s(p1, p1 + dir * 0.5);
                s.DrawArrow(*scene, r / 8, QPen(QBrush(Qt::blue), r / 8));
            }
        }
    }

    // Neighbouring triangles of vertices
    if (verticesNeighboursTriangles)
    {
        double min = r;
        double max = r * 2;
        for (int vi = 0; vi < VertexSize(); ++vi)
        {
            Vector2 p = vertices[vi];
            auto tis = VertexNeighboursTriangles(vi);
            int n = tis.size();
            for (int i = 0; i < n; ++i)
            {
                int ti = tis[i];
                Vector2 c = GetTriangle(ti).Center();
                Vector2 dir = Normalized(c - p);
                double t = Math::Lerp(min, max, i / (double)n);
                Segment2 s(p, p + dir * t);
                s.DrawArrow(*scene, r / 8, QPen(QBrush(Qt::darkCyan), r / 8));
            }
        }
    }

    // Neighbouring vertices of vertices
    if (verticesNeighboursVertices)
    {
        double min = r;
        double max = r * 2;
        for (int vi = 0; vi < VertexSize(); ++vi)
        {
            Vector2 p1 = vertices[vi];
            auto vis = VertexNeighboursVertices(vi);
            int n = vis.size();
            for (int i = 0; i < n; ++i)
            {
                int vin = vis[i];
                Vector2 p2 = vertices[vin];
                Vector2 dir = Normalized(p2 - p1);
                double t = Math::Lerp(min, max, i / (double)n);
                Segment2 s(p1, p1 + dir * t);
                s.DrawArrow(*scene, r / 8, QPen(QBrush(Qt::magenta), r / 8));
            }
        }
    }

    // Points
    for (int vi = 0; vi < VertexSize(); ++vi)
    {
        Vector2 p = vertices[vi];
        if (borders && IsBorderVertex(vi))
            scene->addEllipse(p[0] - r / 4, p[1] - r / 4, 2 * r / 4, 2 * r / 4, QPen(QBrush(Qt::black), r / 10), QBrush(Qt::green));
        else if (borders && IsAloneVertex(vi))
            scene->addEllipse(p[0] - r / 4, p[1] - r / 4, 2 * r / 4, 2 * r / 4, QPen(QBrush(Qt::black), r / 10), QBrush(Qt::blue));
        else
            scene->addEllipse(p[0] - r / 4, p[1] - r / 4, 2 * r / 4, 2 * r / 4, QPen(QBrush(Qt::black), r / 10), QBrush(Qt::red));
    }

    return scene;
}