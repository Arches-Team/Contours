// Noise

#pragma once

#include "noise.h"
#include "evector.h"
#include "matrix.h"

class NoiseTurbulence2 : public Noise2
{
protected:
  double v0 = 0.0; //!< Base value.
  double a0 = 1.0; //!< Base amplitude.
  double l0 = 1.0; //!< Wavelength.
  double alpha = 0.5; //!< Amplitude attenuation coefficient.
  double lambda = 0.5;  //!< Wavelength amplification coefficient.
  int octaves = 8; //!< Number of octaves.
  Vector t = Vector::Null; //!< Translation.
public:
  explicit NoiseTurbulence2(const double& = 0.0, const double& = 1.0, const double& = 1.0, const double& = 0.5, const double& = 0.5, int = 5, const Vector& = Vector::Null);
  ~NoiseTurbulence2();
  double Value(const Vector2&) const;

  using AnalyticScalarField2::Sample;
protected:
  static const Matrix2 R2; //!< %Random incremental rotation in the plane.
};

