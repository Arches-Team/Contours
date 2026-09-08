// Quintics

#pragma once

class Quintic
{
public:

  static double Smooth(const double&);

};

/*!
\brief Compute the value of smooth C<SUP>2</SUP> interpolating function over unit interval.

The quintic is defined as x<SUP>3</SUP>(6 x<SUP>2</SUP>-15 x + 10).
Its first and second derivatives at 0.0 and 1.0 are 0.0.

The Lipschitz constant of the smooth quintic over [0,1] is &lambda;=15/8.

\param x Argument in [0,1].
\sa Septic::Smooth(), Cubic::Smooth()
*/
inline double Quintic::Smooth(const double& x)
{
  return x * x * x * (x * (x * 6.0 - 15.0) + 10.0);
}

