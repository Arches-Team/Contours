// Analytic fields

#include "scalarfield.h"

/*!
\brief Compute the value of the field.
\param p Point.
*/
#pragma warning(push)
#pragma warning(disable: 4100)  
double AnalyticScalarField2::Value(const Vector2& p) const
{
  return 0.0;
}
#pragma warning(pop)

/*!
\brief Create a discrete scalar field by sampling a given domain.
\param a Domain.
*/
ScalarField2 AnalyticScalarField2::Sample(const Array2& a) const
{
  ScalarField2 s(a);
  const int nx = a.GetSizeX();
  const int ny = a.GetSizeY();

  // Set the scalar field values by evaluating the function over the vertices of the grid
  for (int i = 0; i < nx; i++)
  {
    for (int j = 0; j < ny; j++)
    {
      s(i, j) = Value(a.ArrayVertex(i, j));
    }
  }

  return s;
}

//   // Mean
