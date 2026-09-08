
#include "sampling.h"
#include "mesh.h"

const QPoint DiscTileLinked::next[9] = { QPoint(1,0),QPoint(1,1),QPoint(0,1),QPoint(-1,1),QPoint(-1,0),QPoint(-1,-1),QPoint(0,-1),QPoint(1,-1),QPoint(0,0) };
const Vector2 DiscTileLinked::nextvector[9] = { Vector2(1,0), Vector2(1,1),Vector2(0,1),Vector2(-1,1),Vector2(-1,0),Vector2(-1,-1),Vector2(0,-1),Vector2(1,-1),Vector2(0,0) };

/*!
\brief Return the indexes defining the j-th vertex linked to vertex i.
\param i Vertex in the tile.
\param j Link.
\return A QPoint with first coordinate set as the vertex index, and second coordinate as the cell.

*/
QPoint DiscTileLinked::Next(int i, int j) const
{
  int v = topo[i].index[j];
  int q = topo[i].cell[j];

  return QPoint(v, q);
}

/*!
\brief Return the valence of the disc.
\param i Index of the disc in the tile.

*/
int DiscTileLinked::Valence(int i) const
{
  return topo.at(i).n;
}

/*!
\brief Append a topological link to a disc.
\param i Index of the disc.
\param o Octant.
\param d Distance.
*/
inline void DiscTileLinked::DiscLink::AddU(int i, int o, double d)
{
  if (n == size) return;  index[n] = i; cell[n] = o; distanceQXQ[n] = d; n++;
}

/*!
\brief Compute the set of points in the one-neighborhood of the tile that also lie inside the argument box.
\param box The box.
\param q Returned set of points.
\param refs Returned set of references, index and cell.
*/
void DiscTileLinked::TrucMuch(const Box2& box, QVector<Vector2>& q, QVector<QPoint>& refs) const
{
  // Initialize set with points in the tile
  q = p;

  // Set corresponding indexes
  for (int i = 0; i < p.size(); i++)
  {
    refs.append(QPoint(i, 8));
  }

  // Build clones within extended box
  for (int i = 0; i < 8; i++)
  {
    for (int j = 0; j < p.size(); j++)
    {
      Vector2 pji = p.at(j) + nextvector[i] * s;
      if (box.Inside(pji))
      {
        // Add point
        q.append(pji);
        // Keep track of cells
        refs.append(QPoint(j, i));
      }
    }
  }
}

/*
\brief Create a Poisson tile with a Delaunay triangulation connecting neighboring points.

\param s Size of the side of the square.
\param r Radius of the samples.
\param m Maximum number of samples in the dart throwing process.

\sa DiscTile::DiscTile

  */
DiscTileLinked::DiscTileLinked(const double& s, const double& r, int m) : DiscTile(s, r, m)
{
  Relaxation(50);

  Box2 box = GetBox();

  // Extended box
  Box2 extended = box.Extended(6.0 * r);

  QVector<Vector2> vs;
  QVector<QPoint> ids;

  // Get the list of points inside the extended box 
  TrucMuch(extended, vs, ids);

  // Delaunay on extended domain
  Mesh2 mesh = Mesh2::Delaunay(vs);

  topo.resize(p.size());

  // Parse all the vertexes of the Delaunay triangulation
  for (int i = 0; i < mesh.IndexSize(); i++)
  {
    int ma = mesh.index(i);
    int mb = mesh.index(Mesh2::NextIndex(i));

    // Index, cell and point
    int ia = ids.at(ma).x();
    QPoint qa = next[ids.at(ma).y()];
    Vector2 pa = Vertex(qa.x(), qa.y(), ia);

    if (!box.Inside(pa)) continue;

    int ib = ids.at(mb).x();
    int kb = ids.at(mb).y();
    QPoint qb = next[kb];
    Vector2 pb = Vertex(qb.x(), qb.y(), ib);

    double d = Norm(pb - pa);
    topo[ia].AddU(ib, kb, d);

  }
}

