// Sampling  

#pragma once

#include "box.h"
#include "random.h"

class DiscTile
{
protected:
  double s = 0.0; //!< Size.
  double r = 0.0; //!< Radius.
  double e = 0.0; //!< Squared radius with small epsilon tolerance.
  QVector<Vector2> p; //!< Set of samples.
public:
  //! Empty
  explicit DiscTile();
  explicit DiscTile(const double&, const double&, int, Random & = Random::R239);

  //! Empty
  ~DiscTile() {}
  void Relaxation(const double& = 1.0 / 4.0);
  void Relaxation(int, const double& = 1.0 / 4.0);

  Box2 GetBox() const;
  double Radius() const;

  Vector2 Vertex(int) const;
  Vector2 Vertex(int, int, int) const;

  int Size() const;

  void Scale(const double&);

protected:
  void Generate(int, Random & = Random::R239);
};

/*!
\brief Return the box.
*/
inline Box2 DiscTile::GetBox() const
{
  return Box2(Vector2::Null, Vector2(s));
}

/*!
\brief Getter on the i-th sample.
\param i Sample index.
*/
inline Vector2 DiscTile::Vertex(int i) const
{
  return p.at(i);
}

/*!
\brief Compute the i-th sample for a displaced tile.
\param x,y Integer coordinates of the tile.
\param i Sample index.
*/
inline Vector2 DiscTile::Vertex(int x, int y, int i) const
{
  return GetBox().Tile(x, y)[0] + p.at(i);
}

/*!
\brief Return the number of samples.
*/
inline int DiscTile::Size() const
{
  return p.size();
}

/*!
\brief Return the radius.
*/
inline double DiscTile::Radius() const
{
  return r;
}

class DiscTileLinked : public DiscTile
{
protected:
  class DiscLink
  {
  protected:
//     //QVector<QPoint> links; // Store n, and pairs of (index,cell), dynamic
    static constexpr const int size = 32; //!< Maximum number of neighbors
    int index[size] = { -1 }; //!< Neighboring vertex indexes.
    int cell[size] = { -1 }; //!< Neighboring cells.
    double distanceQXQ[size] = { 0.0 }; //!< Distances.
    int n = 0; //!< Number of neighbors.
  public:
    void AddU(int i, int o, double d);
    friend class DiscTileLinked;
  };
  QVector<DiscLink> topo; //!< Topology

public:
  explicit DiscTileLinked(const double&, const double&, int);

  QPoint Next(int, int) const;
  int Valence(int) const;
    
protected:
  void TrucMuch(const Box2&, QVector<Vector2>& q, QVector<QPoint>& refs) const;
protected:
  static const QPoint next[9]; //!< Neigboring cells, including self.
  static const Vector2 nextvector[9]; //!< Offset vectors for neigboring cells, including self.

};

