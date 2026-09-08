// Cubics

#pragma once

class Cubic
{
protected:
  double c[4] = { 0.0,0.0,0.0,0.0 }; //!< %Array of coefficients.
public:
  //! Empty.
  Cubic() {}
  explicit Cubic(const double&, const double&, const double&, const double&);
  ~Cubic() {}

//   // Unary operators

  // Evaluates cubic
  constexpr double operator()(const double&) const;

  static Cubic Hermite(const double&, const double&, const double&, const double&);
  static double Interpolation(const double&, const double&, const double&, const double&, const double&);

  static double SmoothCompact(const double&, const double&);
  static double Smooth(const double&);

};

/*!
\brief Creates a cubic.

Coefficients start from the highest degree to the lowest.
\param a, b, c, d Coefficients of the cubic a x<SUP>3</SUP>+b x<SUP>2</SUP>+c x+d.
*/
inline Cubic::Cubic(const double& a, const double& b, const double& c, const double& d) 
{
  Cubic::c[3] = a;
  Cubic::c[2] = b;
  Cubic::c[1] = c;
  Cubic::c[0] = d;
}

/*!
\brief Evaluates the cubic.

\param x Value.
*/
inline constexpr double Cubic::operator()(const double& x) const
{
  return c[0] + x * (c[1] + x * (c[2] + x * c[3]));
}

/*!
\brief Compactly supported smooth interpolating function.

\sa Cubic::Smooth(const double&, const double&), Quadric::SmoothCompact()

\param x Squared distance.
\param r Squared radius.
*/
inline double Cubic::SmoothCompact(const double& x, const double& r)
{
  return (x > r) ? 0.0 : (1.0 - x / r) * (1.0 - x / r) * (1.0 - x / r);
}

/*!
\brief Compute the value of a C<SUP>1</SUP> smooth interpolating function.

The cubic is defined as x<SUP>2</SUP>(3 - 2 x). Its first derivatives at 0.0 and 1.0 are 0.0.

The Lipschitz constant of the smooth quintic over [0,1] is &lambda;=3/2.

\param x Argument in [0,1].

\sa Quintic::Smooth(), Septic::Smooth()
*/
inline double Cubic::Smooth(const double& x)
{
  return x * x * (3.0 - 2.0 * x);
}

