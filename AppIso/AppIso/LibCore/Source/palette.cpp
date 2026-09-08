// Palette 

#include "palette.h"
#include "linear.h"

/*!
\brief Create an empty palette.
*/
Palette::Palette()
{
}

/*!
\brief Create a palette.
\param c Set of evenly distributed colors points.
*/
Palette::Palette(const QVector<Color>& c) :c(c)
{
  // Indexes
  for (int i = 0; i < c.size(); i++)
  {
    a.append(Math::Unit(i, c.size()));
  }
}

/*!
\brief Create a palette.

This constructeur allows to create a simple color ramp easily:
\code
  Palette palette({ Color(0.5),Color(1.0) }); // Grey to white
\endcode

\param qtc Set of evenly distributed Qt colors points.
*/
Palette::Palette(const QVector<QColor>& qtc)
{
  for (int i = 0; i < c.size(); i++)
  {
    c.append(Color(qtc.at(i)));
  }

  // Indexes
  for (int i = 0; i < qtc.size(); i++)
  {
    a.append(Math::Unit(i, qtc.size()));
  }
}

/*!
\brief Compute a color in the palette.
\param t Interpolation parameter.
*/
Color Palette::GetColor(double t) const
{
  // Diverging palette with three colors over unit interval
  if (type == 1)
  {
    if (t < 0.5)
    {
      return Color::Lerp(2.0 * t, c.at(0), c.at(1));
    }
    else
    {
      return Color::Lerp(2.0 * t - 1.0, c.at(1), c.at(2));
    }
  }

  // General case
  if (c.size() == 0)
    return Color::White;

  if (c.size() == 1)
    return c.at(0);

  if (t < a.at(0))
    return c.at(0);

  if (t >= a.at(a.size() - 1))
    return c.at(c.size() - 1);

  for (int i = 0; i < c.size() - 1; i++)
  {
    if (t < a.at(i + 1))
    {
      double s = Linear::Step(t, a.at(i), a.at(i + 1));
      return Color::Lerp(s, c.at(i), c.at(i + 1));
    }
  }
  return Color::White;
}

