 // Arrays

 #include "array.h"

const QPoint Array2::next[8] = { QPoint(1, 0), QPoint(1, 1), QPoint(0, 1), QPoint(-1, 1), QPoint(-1, 0), QPoint(-1, -1), QPoint(0, -1), QPoint(1, -1) };
const double Array2::inverselength[8] = { 1.0, 1.0 / sqrt(2.0), 1.0, 1.0 / sqrt(2.0), 1.0, 1.0 / sqrt(2.0), 1.0, 1.0 / sqrt(2.0) };

/*!
\brief Empty array, with empty box.
*/
Array2::Array2() :Box2(Box2::Null), nx(0), ny(0), celldiagonal(Vector2::Null), inversecelldiagonal(Vector2::Null)
{
}

/*!
\brief Create the lattice structure.
\param box The box.
\param x,y Size of the lattice.
*/
Array2::Array2(const Box2& box, int x, int y) :Box2(box), nx(x), ny(y), celldiagonal((b[0] - a[0]) / (nx - 1), (b[1] - a[1]) / (ny - 1))
{
  inversecelldiagonal = celldiagonal.Inverse();
}

/*!
\brief Compute the k-th point on the boundary of the domain.

\image html borders.png

\param k Integer index.
*/
QPoint Array2::VertexBorder(int k) const
{
  int x;
  int y;
  if (k < 2 * nx)
  {
    if (k < nx)
    {
      x = k;
      y = 0;
    }
    else
    {
      x = k - nx;
      y = ny - 1;
    }
  }
  else
  {
    k -= 2 * nx;
    if (k < ny - 2)
    {
      x = 0;
      y = k + 1;
    }
    else
    {
      x = nx - 1;
      y = k + 1 - (ny - 2);
    }
  }

  return QPoint(x, y);
}

/*!
\brief Compute the index of a point on the boundary of the domain.
\param i,j Coordinates.
*/
int Array2::VertexBorderIndex(int i, int j) const
{
  if (j == 0)
  {
    return i;
  }
  else if (j == ny - 1)
  {
    return i + nx;
  }
  else
  {
    // Already processed corners in previous if statements
    if (i == 0)
    {
      return nx * 2 + j - 1;
    }
    else
    {
      return nx * 2 + ny - 2 + j - 1;
    }
  }
}

/*!
\brief Return the center of the cell.
\param i,j Integer coordinates of the cell.
*/
Vector2 Array2::CellCenter(int i, int j) const
{
  return  a + celldiagonal.Scaled(Vector2(i, j)) + 0.5 * celldiagonal;
}

/*!
\brief Return the cell diagonal vector.
*/
Vector2 Array2::CellDiagonal() const
{
  return celldiagonal;
}

