
// Vector  

#include "evector.h"

const Vector Vector::Null = Vector(0.0, 0.0, 0.0);
const Vector Vector::X = Vector(1.0, 0.0, 0.0);
const Vector Vector::Y = Vector(0.0, 1.0, 0.0);
const Vector Vector::Z = Vector(0.0, 0.0, 1.0);

/*!
\brief Normalize a vector, computing the inverse of its norm and scaling
the components.

This function does not check if the vector is null,
which might resulting in errors.
*/
void Normalize(Vector& u)
{
  u *= 1.0 / Norm(u);
}

