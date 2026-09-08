// Polygons

#include "polygon.h"

/*!
\brief Empty.
*/
Polygons2::Polygons2()
{
}

/*!
\brief Add a polygon to the set.
\param p %Polygon.
*/
void Polygons2::Add(const Polygon2& p)
{
  poly.append(p);
}

