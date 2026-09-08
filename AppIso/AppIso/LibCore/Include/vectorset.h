// Vector sets

#pragma once

#include "box.h"

// Sphere class
class VectorSet2
{
protected:
  QVector<Vector2> v; //!< Vectors.
public:
  //! Empty.
  VectorSet2() {}
  VectorSet2(const QVector<Vector2>&);

  //! Empty.
  ~VectorSet2() {}

  VectorSet2 VectorToPoint() const;
  void Reverse();
  
  Vector2 At(int) const;

  Box2 GetBox() const;

  void Translate(const Vector2&);
  void Scale(const double&);
  void Scale(const Vector2&);

  QVector<Vector2>& Get();

};

/*!
\brief Access to element.
*/
inline Vector2 VectorSet2::At(int i) const
{
  return v.at(i);
}

/*!
\brief Access to the internal structure.
*/
inline QVector<Vector2>& VectorSet2::Get()
{
  return v;
}

