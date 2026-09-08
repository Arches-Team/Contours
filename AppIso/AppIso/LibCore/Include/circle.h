// Circle

#pragma once

#include <QtWidgets/QGraphicsScene>

#include "evector.h"
#include "ray.h"

class Circle2
{
protected:
  Vector2 c = Vector2::Null; //!< Center.
  double r = 1.0; //!< Radius.
public:
  //! Empty
  Circle2() {}
  explicit Circle2(const Vector2&, const double&);
  explicit Circle2(const Vector2&, const Vector2&, const Vector2&);

  bool Inside(const Vector2&) const;

  Vector2 Center() const;

  double Radius() const;

  void Extend(const Vector2&);

  bool Intersect(const Ray2&, double&, double&) const;

//   // Drawing
  void Draw(QGraphicsScene&, const QPen & = QPen(), const QBrush & = QBrush()) const;
protected:
  static constexpr const double Epsilon = 1.e-6; //!< %Epslion for intersection tests
};

//! Center of the circle.
inline Vector2 Circle2::Center() const
{
  return c;
}

//! Radius of the circle.
inline double Circle2::Radius() const
{
  return r;
}

