// Matrix 

#include "matrix.h"

/*!
\brief Identity matrix.
*/
const Matrix Matrix::Identity(1.0);

/*!
\brief Multiplication.

\param u, v Input matrices.
*/
Matrix operator*(const Matrix& u, const Matrix& v)
{
  return Matrix(u[0] * v[0] + u[3] * v[1] + u[6] * v[2], u[1] * v[0] + u[4] * v[1] + u[7] * v[2], u[2] * v[0] + u[5] * v[1] + u[8] * v[2],
    u[0] * v[3] + u[3] * v[4] + u[6] * v[5], u[1] * v[3] + u[4] * v[4] + u[7] * v[5], u[2] * v[3] + u[5] * v[4] + u[8] * v[5],
    u[0] * v[6] + u[3] * v[7] + u[6] * v[8], u[1] * v[6] + u[4] * v[7] + u[7] * v[8], u[2] * v[6] + u[5] * v[7] + u[8] * v[8]);
}
