// Curve
#pragma once

#include "vectorset.h"

class PointCurve2 : public VectorSet2
{
protected:
   bool closed = false; //!< Closed curve flag.
public:
  //! Empty.
  PointCurve2() {}
  explicit PointCurve2(const QVector<Vector2>&, bool = false);
  explicit PointCurve2(const QVector<Vector2>&, const QVector<int>&, bool = false);

  //! Empty.
  ~PointCurve2() {}

  // Projection
  double R(const Vector2&, double&, int&) const;

};

