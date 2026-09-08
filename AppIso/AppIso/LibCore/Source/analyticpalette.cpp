// Analytic palettes 

#include "palette.h"
#include "linear.h"

/*!
\brief Compute the color.

\param u %Palette entry.
*/
#pragma warning(push)
#pragma warning(disable: 4100)  
Color GenericPalette::GetColor(double u) const
{
  return Color::Black;
}
#pragma warning(pop)

/*!
\brief Create a palette.
\param n Palette identifier.
\param r Reverse flag.
*/
AnalyticPalette::AnalyticPalette(int n, bool r) :n(n), r(r)
{
}

/*!
\brief Changer the order of the palette.
\param u Interpolation parameter.
*/
inline double AnalyticPalette::Reverse(double u) const
{
  return r == false ? u : 1.0 - u;
}

/*!
\brief Compute color for a diverging palette with three colors.
\param a,b,c %Colors.
\param t Interpolation parameter.
*/
inline Color AnalyticPalette::Diverging(const Color& a, const Color& b, const Color& c, double t)
{
  if (t < 0.5)
  {
    return Color::Lerp(2.0 * t, a, c);
  }
  else
  {
    return Color::Lerp(2.0 * t - 1.0, c, b);
  }
}

/*!
\brief Compute the color.

\param u %Palette entry.
*/
Color AnalyticPalette::GetColor(double u) const
{
  u = Reverse(u);

  switch (n)
  {
  case 0:
    return BrownGreyGreen(u);
    break;
  case 1:
    return GreenBrownGrey(u);
    break;
  case 2:
    return MatlabJet(u);
    break;
  case 3:
    return CoolWarm(u);
    break;
  case 4:
    return WhiteRed(u);
    break;
  case 5:
    return BlueGreen(u);
    break;
  case 6:
    return BlueGreyBrown(u);
    break;
  case 7:
    return GeologyGreenYellow(u);
    break;
  case 8:
    return GeologyGreenYellow2(u);
  case 9:
    return BrownGreen(u);
    break;
  case 10:
    return WhiteBlue(u);
    break;
  case 11:
    return WhiteBrown(u);
    break;
  case 12:
    return GreenOrange(u);
    break;
  }
  return Color::Black;
}

/*!
\brief Custom divergent palette.

Compute a linear interpolation between brown, grey and green.
\param u Interpolant.
*/
Color AnalyticPalette::BrownGreyGreen(double u)
{
    static const Color Brown = Color(153, 93, 18);
    static const Color Green = Color(12, 112, 104);
    static const Color Grey = Color(244, 244, 244);

    return Diverging(Brown, Green, Grey, u);
}

/*!
\brief Green to brown coloring.
\param u %Palette entry.
*/
Color AnalyticPalette::GreenBrownGrey(double u)
{
    static const Color Brown(167, 159, 118);
    static const Color Green(77, 97, 73);
    static const Color Tan(126, 128, 84);
    static const Color Grey(202, 195, 191);

    if (u < 0.33)
    {
        return Color::Lerp(u / 0.33, Green, Tan);
    }
    else if (u < 0.67)
    {
        return Color::Lerp((u - 0.33) / 0.33, Tan, Brown);
    }
    else
    {
        return Color::Lerp((u - 0.67) / 0.33, Brown, Grey);
    }
}

/*!
\brief Diverging palette from blue to red.

\param u %Palette entry.
*/
Color AnalyticPalette::CoolWarm(double u)
{
    static const Color Cool(97, 130, 234);
    static const Color White(221, 221, 221);
    static const Color Warm(220, 94, 75);

    if (u < 0.5)
    {
        return Color::Lerp(u / 0.5, Cool, White);
    }
    else
    {
        return Color::Lerp((u - 0.5) / 0.5, White, Warm);
    }
}

/*!
\brief Utility function for AnalyticPalette::MatlabJet
\author Mathieu Gaillard
*/
double MatlabJetBase(double val)
{
    if (val <= 0.125)
    {
        return 0.0;
    }
    else if (val <= 0.375)
    {
        return Linear::Step(val, 0.125, 0.375, 0.0, 1.0);
    }
    else if (val <= 0.625)
    {
        return 1.0;
    }
    else if (val <= 0.875)
    {
        return Linear::Step(val, 0.625, 0.875, 1.0, 0.0);
    }
    else
    {
        return 0.0;
    }
}

/*!
\brief Equivalent of the Jet coloring in Matlab.
\author Mathieu Gaillard
\param u %Palette entry.
*/
Color AnalyticPalette::MatlabJet(double u)
{
    double r = MatlabJetBase(u - 0.25);
    double g = MatlabJetBase(u);
    double b = MatlabJetBase(u + 0.25);

    return Color(r, g, b);
}

/*!
\brief Blue to green smooth palette.
\param u %Palette entry.
*/
Color AnalyticPalette::BlueGreen(double u)
{
    double r = 0.0;
    double g = 0.215 + 0.715 * u;
    double b = 0.825 - 0.625 * u;

    return Color(r, g, b);
}

/*!
\brief White to red.
\param u %Palette entry.
*/
Color AnalyticPalette::WhiteRed(double u)
{
    return Color::Lerp(u, Color::White, Color(1.0, 0.0, 0.0));
}

/*!
\brief Custom divergent palette.

Inspired by Kenneth Moreland's diverging color map.

\image html palette-6.png

Compute a linear interpolation between blue, grey, and brown.
\param u Interpolant.
*/
Color AnalyticPalette::BlueGreyBrown(double u)
{
    static const Color Blue = Color(0.247, 0.522, 0.937);
    static const Color Grey = Color(0.863);
    static const Color Brown = Color(0.67, 0.502, 0.000);

    return Diverging(Blue, Brown, Grey, u);
}

/*!
\brief Custom divergent palette.

\param u Interpolant.
*/
Color AnalyticPalette::GeologyGreenYellow(double u)
{
    static const Color Cool(34.0 / 255.0, 52.0 / 255.0, 63.0 / 255.0);
    static const Color White(108.0 / 255.0, 184.0 / 255.0, 117.0 / 255.0);
    static const Color Warm(236.0 / 255.0, 237.0 / 255.0, 105 / 255.0);

    return Diverging(Cool, Warm, White, u);
}

/*!
\brief Custom divergent palette.

\param u Interpolant.
*/
Color AnalyticPalette::GeologyGreenYellow2(double u)
{
    static const Color White(108.0 / 255.0, 184.0 / 255.0, 117.0 / 255.0);
    static const Color Warm(236.0 / 255.0, 237.0 / 255.0, 105 / 255.0);
    return Color::Lerp(u, White, Warm);
}

/*!
\brief Diverging palette from brown to green.
*/
Color AnalyticPalette::BrownGreen(double u)
{
    static const Color Brown(153, 93, 18);
    static const Color White(244, 244, 244);
    static const Color Green(12, 112, 104);

    return Diverging(Brown, Green, White, u);
}

/*!
\brief White to blue.
\param u %Palette entry.
*/
Color AnalyticPalette::WhiteBlue(double u)
{
  return Color::Lerp(u, Color::White, Color(97, 130, 234));
}

Color AnalyticPalette::WhiteBrown(double u)
{
  return Color::Lerp(u, Color(1.0), Color(158 / 255.0, 137 / 255.0, 114 / 255.0));
}

Color AnalyticPalette::GreenOrange(double u)
{
  static const Color Orange(247 / 255.0, 150 / 255.0, 70 / 255.0);
  static const Color Green(146 / 255.0, 208 / 255.0, 80 / 255.0);

  if (u < 0.5)
    return Color::Lerp(u / 0.5, Orange, Color::White);
  else
    return Color::Lerp((u - 0.5) / 0.5, Color::White, Green);
}

