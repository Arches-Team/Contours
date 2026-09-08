// Linear polynomials

#pragma once

class Linear
{
public:

  static double Step(const double&, const double&, const double&);
  static double Step(const double&, const double&, const double&, const double&, const double&);

};

/*!
\brief Create a linear step.
\param x Value
\param a, b Interval values.
\return Real in unit inverval.
\sa Cubic::SmoothStep, Quintic::SmoothStep
*/
inline double Linear::Step(const double& x, const double& a, const double& b)
{
  if (x < a)
  {
    return 0.0;
  }
  else if (x > b)
  {
    return 1.0;
  }
  else
  {
    return (x - a) / (b - a);
  }
}

/*!
\brief Create a linear step.

This function is the same as coding:
\code
double y=Math::Lerp(ya,yb,Linear::Step(x,a,b));
\endcode
\param x Value
\param a, b Interval values.
\param ya, yb Output interval values.
\sa Cubic::SmoothStep, Quintic::SmoothStep
*/
inline double Linear::Step(const double& x, const double& a, const double& b, const double& ya, const double& yb)
{
  if (x < a)
  {
    return ya;
  }
  else if (x > b)
  {
    return yb;
  }
  else
  {
    return ya + (x - a) * (yb - ya) / (b - a);
  }
}

