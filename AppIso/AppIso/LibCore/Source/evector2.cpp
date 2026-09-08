// Vector  
#include "evector.h"

const Vector2 Vector2::Null = Vector2(0.0, 0.0);

const Vector2 Vector2::X = Vector2(1.0, 0.0);

/*!
\brief Swap two vectors.
\param a, b Vectors.
*/
void Swap(Vector2& a, Vector2& b)
{
  Vector2 t = a;
  a = b;
  b = t;
}
