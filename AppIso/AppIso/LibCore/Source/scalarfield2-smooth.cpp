
// Fields

#include "scalarfield.h"

/*!
\brief Applies several smoothing steps to the scalar field.

\sa Smooth()
\param n Number of smoothing steps.
*/
void ScalarField2::Smooth(int n)
{
  for (int i = 0; i < n; i++)
  {
    Smooth();
  }
}

/*!
\brief Smooth the scalar field using a discrete gaussian kernel.

The function uses a 3<SUP>2</SUP> approximation of the Gaussian kernel.
*/
void ScalarField2::Smooth()
{
  QVector<double> smoothed;
  smoothed.resize(nx * ny);

  int k;

  // Smooth center
  for (int i = 1; i < nx - 1; i++)
  {
    for (int j = 1; j < ny - 1; j++)
    {
      k = VertexIndex(i, j);
      smoothed[k] = (4.0 * at(k) + 2.0 * at(k - 1) + 2.0 * at(k + 1) + 2.0 * at(k - nx) + 2.0 * at(k + nx) + at(k - 1 - nx) + at(k + 1 - nx) + at(k - 1 + nx) + at(k + 1 + nx)) / 16.0;
    }
  }

  // Smooth edges
  for (int i = 1; i < nx - 1; i++)
  {
    k = VertexIndex(i, 0);
    smoothed[k] = (4.0 * at(k) + 2.0 * at(k - 1) + 2.0 * at(k + 1) + 2.0 * at(k + nx) + at(k - 1 + nx) + at(k + 1 + nx)) / 12.0;

    k = VertexIndex(i, ny - 1);
    smoothed[k] = (4.0 * at(k) + 2.0 * at(k - 1) + 2.0 * at(k + 1) + 2.0 * at(k - nx) + at(k - 1 - nx) + at(k + 1 - nx)) / 12.0;
  }

  for (int j = 1; j < ny - 1; j++)
  {
    k = VertexIndex(0, j);
    smoothed[k] = (2.0 * at(k - nx) + 4.0 * at(k) + 2.0 * at(k + nx) + at(k + 1 - nx) + 2.0 * at(k + 1) + at(k + 1 + nx)) / 12.0;

    k = VertexIndex(nx - 1, j);
    smoothed[k] = (2.0 * at(k - nx) + 4.0 * at(k) + 2.0 * at(k + nx) + at(k - 1 - nx) + 2.0 * at(k - 1) + at(k - 1 + nx)) / 12.0;
  }

  // Corners
  k = VertexIndex(0, 0);
  smoothed[k] = (4.0 * at(k) + 2.0 * at(k + 1) + 2.0 * at(k + nx) + 1.0 * at(k + nx + 1)) / 9.0;

  k = VertexIndex(nx - 1, 0);
  smoothed[k] = (4.0 * at(k) + 2.0 * at(k - 1) + 2.0 * at(k + nx) + 1.0 * at(k + nx - 1)) / 9.0;

  k = VertexIndex(0, ny - 1);
  smoothed[k] = (4.0 * at(k) + 2.0 * at(k + 1) + 2.0 * at(k - nx) + 1.0 * at(k - nx + 1)) / 9.0;

  k = VertexIndex(nx - 1, ny - 1);
  smoothed[k] = (4.0 * at(k) + 2.0 * at(k - 1) + 2.0 * at(k - nx) + 1.0 * at(k - nx - 1)) / 9.0;

  // Center
  field = smoothed;
}
