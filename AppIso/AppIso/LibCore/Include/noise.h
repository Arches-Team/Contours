// Noise

#pragma once

#include "scalarfield.h"

class Noise2 : public AnalyticScalarField2
{
protected:
  bool quintic = false; //!< Flag defining interpolation type: either cubic (false) or quintic (true).
public:
  Noise2(bool = false);
  //! Empty
  ~Noise2() {}

  double Value(const Vector2&) const;

protected:
  static double rtable[267]; //!< Random double table.
  static const short hashTable[4096]; //!< Hash table.
protected:
  // Hash code
  short Hash1d(int, int) const;
  short Hash2d(int, int) const;

  double GradientVertex(double*, double, double) const;
};

