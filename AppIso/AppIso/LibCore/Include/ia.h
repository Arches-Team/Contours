// Interval arithmetic

#pragma once

#include "mathematics.h"

class Ia {
private:
  double a = 0.0, b = 0.0; //!< Interval bounds.
public:
  //! Empty 
  Ia() {}
  explicit Ia(const double&);
  explicit Ia(const double&, const double&);

  // Access Ia
  constexpr double& operator[] (int);
  constexpr double operator[] (int) const;

  bool operator<(const double&) const;

};

/*!
\brief Creates an interval.
\param x, y Low and high real values.
*/
inline Ia::Ia(const double& x, const double& y) :a(x), b(y)
{
}

/*!
\brief Creates an interval.

The length of the interval is 0.
\param x Real.
*/
inline Ia::Ia(const double& x) :a(x), b(x)
{
}

//! Access interval bounds.
inline constexpr double& Ia::operator[] (int i)
{
  if (i == 0) return a;
  else return b;
}

//! Access interval bounds.
inline constexpr double Ia::operator[] (int i) const
{
  if (i == 0) return a;
  else return b;
}
