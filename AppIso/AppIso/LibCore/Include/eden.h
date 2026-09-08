// Eden growth

#pragma once

#include "evector.h"
#include "circle.h"
#include "particle.h"
#include "random.h"

class EdenGrowth2 : public ParticleSet2
{
protected:
  Circle2 circle; //!< Bounding circle of the domain.
  QVector<int> v; //!< Tree structure topology, <I>i.e.</I>, index of the ancestor particle during growth.
  QVector<int> s; //!< Set of indexes referencing boundary cells.
  int SamplingN = 37; //!< Sampling: generate N evenly distributed cells around a given cell, with angle 2&pi;/N, 37 approximate off lattice, 4 and 6 are the square and hexagonal lattices respectively.
public:
  explicit EdenGrowth2(const Vector2&, const double&, int = 37);

  //! Empty.
  ~EdenGrowth2() {}

  virtual bool Collide(const Vector2&) const;

  virtual void Step();
  virtual void Step(int);

  QVector<int> GetIndices() const;

protected:
  virtual int GrowthSites(const Vector2&, Vector2*) const;
  virtual int Select() const;
protected:
  static Random random; //!< %Random number generator.
};

