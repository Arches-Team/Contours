// Color

#pragma once

// Qt Color
#include <QtGui/QColor>

// Mathematics fundamentals
#include "mathematics.h"

class Color
{
protected:
  double c[4] = { 0.0,0.0,0.0,1.0 }; //!< %Array of color components, with an alpha channel.
public:
  explicit Color(const double& = 0.0);
  explicit Color(const double&, const double&, const double&, const double& = 1.0);
  explicit Color(int, int, int, int = 255);
  explicit Color(const QColor&);

  static Color Lerp(const double&, const Color & = Color::Black, const Color & = Color::White);

  constexpr double& operator[] (int);
  constexpr double operator[] (int) const;

   QColor GetQt() const;

  static const Color Grey(const double&);
  static const Color Black; //!< %Black.
  static const Color White; //!< %White.

};

/*!
\brief Returns the i<sup>th</sup> channel of the spectrum
\param i nummber of the channel queried (default=last)
*/
inline constexpr double& Color::operator[] (int i)
{
  return c[i];
}

/*!
\brief Returns a copy of the i<sup>th</sup> channel of the spectrum
\param i nummber of the channel queried (default=last)
*/
inline constexpr double Color::operator[] (int i) const
{
  return c[i];
}

/*!
\brief %Linear interpolation between two colors.

Interpolation is performed in RGB space.

Note that the following lines are equivalent:
\code
Color a,b;
double t;
Color c=(1.0-t)*a+t*b;
Color c=Lerp(t,a,b);
\endcode
*/
inline Color Color::Lerp(const double& t, const Color& a, const Color& b)
{
  return Color((1.0 - t) * a[0] + t * b[0], (1.0 - t) * a[1] + t * b[1], (1.0 - t) * a[2] + t * b[2], (1.0 - t) * a[3] + t * b[3]);
}

/*!
\brief Creates a greyscale color.

Initializes all the components to the given value except the opacity coefficient wich is set to 1.0.
\param v Grey value.
*/
inline Color::Color(const double& v) :Color(v, v, v, 1.0)
{
}

/*!
\brief Creates a color given each of these components
\param r,g,b Red, green and blue components.
\param a Alpha channel, set to 1.0 (opaque) as default.
*/
inline Color::Color(const double& r, const double& g, const double& b, const double& a)
{
  c[0] = r;
  c[1] = g;
  c[2] = b;
  c[3] = a;
}

/*!
\brief Create a color given integer components
\param r, g, b Red, green and blue components.
\param a Alpha channel, set to 255 (opaque) as default.
*/
inline Color::Color(int r, int g, int b, int a) :Color(r / 255.0, g / 255.0, b / 255.0, a / 255.0)
{
}

/*!
\brief Create a Qt color from a color.

Example how to write a pixel with a given color in an image:
\code
QImage image;
Color c;
image.setPixel(i, j, c.GetQt().rgb());
\endcode
*/
inline QColor Color::GetQt() const
{
  return QColor(int(255.0 * Math::Clamp(c[0])), int(255.0 * Math::Clamp(c[1])), int(255.0 * Math::Clamp(c[2])), int(255.0 * Math::Clamp(c[3])));
}

/*!
\brief Grey.

Same as Color(const double&)

\param g Grey level.
*/
inline const Color Color::Grey(const double& g)
{
  return Color(g);
}

