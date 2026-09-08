// Fundamentals

#ifndef DBL_MAX
#define DBL_MAX        1.7976931348623158e+308
#define DBL_MIN        2.2250738585072014e�308
#endif

#pragma once

#include <math.h>

#include <iostream>

// Math class for constants
class Math
{
public:
  static constexpr double Pi = 3.14159265358979323846; //!< &pi;.
  static constexpr double TwoPi = 6.28318530717958647693;; //!< 2&pi;.
  static constexpr double Infinity = DBL_MAX; //!< Infinity.

public:
  static double Clamp(const double&, const double& = 0.0, const double& = 1.0);
  static int Clamp(int, int = 0, int = 255);

  // Squares
  static constexpr double Sqr(const double&);
  static double Pow(const double&, const double&);
  static double Abs(const double&);

  // Modulo
  static constexpr double Mod(const double&, const double&);

  static double Floor(const double&);
  static double Ceil(const double&);

  // Minimum and maximum
  static double Min(const double&, const double&);
  static double Max(const double&, const double&);
  static double Max(const double&, const double&, const double&);
  static double Min(const double&, const double&, const double&, const double&);
  static double Max(const double&, const double&, const double&, const double&);

  // Angles
  static constexpr double DegreeToRadian(const double&);
  static constexpr double RadianToDegree(const double&);

  static constexpr double Angle(int, int);

  // Linear interpolation
  static double Lerp(const double&, const double&, const double&);
  static double Bilinear(const double&, const double&, const double&, const double&, const double&, const double&);

  static void SetMinMax(const double&, double&, double&);

  static int Integer(const double&);

  static void Swap(int&, int&);
  static void Swap(double&, double&);

  static constexpr double Unit(int, int);

};

/*!
\brief Bi-linear interpolation between four values.

The values are given in trigonometric order.

\image html bilinear.png
\param a00, a10, a11, a01 Interpolated values.
\param u,v Interpolation coefficients.
*/
inline double Math::Bilinear(const double& a00, const double& a10, const double& a11, const double& a01, const double& u, const double& v)
{
  return (1.0 - u) * (1.0 - v) * a00 + (1.0 - u) * (v)*a01 + (u) * (1.0 - v) * a10 + (u) * (v)*a11;
}

/*!
\brief Squares a double value.
\sa Math::SymmetricSqr
\param x Real value.
*/
inline constexpr double Math::Sqr(const double& x)
{
  return x * x;
}

/*!
\brief Linear interpolation.

Returns (1-t)a+tb.

\param a,b Interpolated values.
\param t Interpolant.
*/
inline double Math::Lerp(const double& a, const double& b, const double& t)
{
  return a + t * (b - a);
}

/*!
\brief Convert degrees to randians.
\param a Angle in degrees.
*/
inline constexpr double Math::DegreeToRadian(const double& a)
{
  return a * Math::Pi / 180.0;
}

/*!
\brief Convert radian to degrees.
\param a Angle in radian.
*/
inline constexpr double Math::RadianToDegree(const double& a)
{
  return a * 180.0 / Math::Pi;
}

/*!
\brief Compute 2 k &pi; / n.
\param k, n Integers.
*/
inline constexpr double Math::Angle(int k, int n)
{
  return (Math::TwoPi * k) / n;
}

/*!
\brief Swap two reals.
\sa Sort(double&,double&)
\param a, b Arguments.
*/
inline void Math::Swap(double& a, double& b)
{
  double t = a;
  a = b;
  b = t;
}

/*!
\brief Swap two integers.
\sa Sort(int&,int&)
\param a, b Arguments.
*/
inline void Math::Swap(int& a, int& b)
{
  int t = a;
  a = b;
  b = t;
}

/*!
\brief Compute the integer part of a real.

This function handles negative values differently by subtracting 1 from the result.
\param x %Real.
*/
inline int Math::Integer(const double& x)
{
  return (x > 0.0 ? int(x) : int(x) - 1);
}

/*!
\brief Modulus for reals with negative values handled properly.
\param x %Real.
\param a Modulo.
*/
inline constexpr double Math::Mod(const double& x, const double& a)
{
  return (x >= 0.0) ? fmod(x, a) : a - fmod(-x, a);
}

/*!
\brief Clamp a double value between two bounds.
\param x Input value.
\param a, b Lower and upper bounds.
*/
inline double Math::Clamp(const double& x, const double& a, const double& b)
{
  return (x < a ? a : (x > b ? b : x));
}

/*!
\brief Clamp an integer value between two bounds.
\param x Input value.
\param a, b Lower and upper bounds.
*/
inline int Math::Clamp(int x, int a, int b)
{
  return (x < a ? a : (x > b ? b : x));
}

/*!
\brief Update the minimum and maximum values given a double value.
\param x Input value.
\param a, b Lower and upper bounds that will be updated according to x.
*/
inline void Math::SetMinMax(const double& x, double& a, double& b)
{
  /*
  if (x < a)
  {
    a = x;
  }
  else if (x > b)
  {
    b = x;
  }
  */
  a = Math::Min(a, x);
  b = Math::Max(b, x);
}

/*!
\brief Minimum of two reals.
\param a, b Real values.
*/
inline double Math::Min(const double& a, const double& b)
{
  return (a < b ? a : b);
}

/*!
\brief Maximum of two reals.
\param a, b Real values.
*/
inline double Math::Max(const double& a, const double& b)
{
  return (a > b ? a : b);
}

/*!
\brief Maximum of three reals.
\param a, b, c Real values.
*/
inline double Math::Max(const double& a, const double& b, const double& c)
{
  return Math::Max(Math::Max(a, b), c);
}

/*!
\brief Maximum of four reals.
\param a, b, c, d Real values.
*/
inline double Math::Max(const double& a, const double& b, const double& c, const double& d)
{
  return Math::Max(Math::Max(a, b), Math::Max(c, d));
}

/*!
\brief Minimum of four reals.
\param a, b, c, d Real values.
*/
inline double Math::Min(const double& a, const double& b, const double& c, const double& d)
{
  return Math::Min(Math::Min(a, b), Math::Min(c, d));
}

/*!
\brief %Floor function.

While it is easier to use the C++ function, this has been implemented to be consistent with Fract().
\param x Real.
*/
inline double Math::Floor(const double& x)
{
  return floor(x);
}

/*!
\brief %Ceil function.
\sa Floor
\param x Real.
*/
inline double Math::Ceil(const double& x)
{
  return ceil(x);
}

/*!
\brief Power.

Returns base x raised to the power exponent e if x>0, and -x raised to the power exponent e otherwise.

\param x Base.
\param ee Exponent.
*/
inline double Math::Pow(const double& x, const double& ee)
{
  if (x == 0.0)
  {
    return 0.0;
  }
  else if (x > 0.0)
  {
    return pow(x, ee);
  }
  else
  {
    return -pow(-x, ee);
  }
}

/*!
\brief Absolute value.

\param x Real.
*/
inline double Math::Abs(const double& x)
{
  return fabs(x);
}

/*!
\brief Unit real value in [0,1] from two integers.

Usefull for loops, same as:
\code
for (int i=0;i<n;i++)
{
double t=double(i)/double(n-1); // t=Math::Unit(i,n);
}
\endcode
\param i, n Integers.
*/
inline constexpr double Math::Unit(int i, int n)
{
  return double(i) / double(n - 1);
}

