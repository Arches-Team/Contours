#pragma once
// Vector

// Mathematics fundamentals
#include "mathematics.h"

// Class
class Vector
{
protected:
  double c[3] = { 0.0, 0.0, 0.0 }; //!< Components.
public:
  Vector() {}

  explicit Vector(const double&);
  explicit Vector(const double&, const double&, const double&);

  // Access members
  constexpr double& operator[] (int);
  constexpr double operator[] (int) const;

  Vector operator- () const;

//   // Assignment operators
  Vector& operator+= (const Vector&);
  Vector& operator*= (const double&);
  Vector& operator/= (const double&);

  // Binary operators
  friend Vector operator+ (const Vector&, const Vector&);
  friend Vector operator- (const Vector&, const Vector&);

  friend Vector operator* (const Vector&, double);
  friend Vector operator* (const double&, const Vector&);
  friend Vector operator/ (const Vector&, double);

  friend Vector operator/ (const Vector&, const Vector&);

  // Norm
  friend double Norm(const Vector&);

  double Max() const;

  friend void Normalize(Vector&);
  friend Vector Normalized(const Vector&);

public:
  static const Vector Null; //!< Null vector.
  static const Vector X; //!< Vector(1,0,0).
  static const Vector Y; //!< Vector(0,1,0).
  static const Vector Z; //!< Vector(0,0,1).
};

/*!
\brief Create a vector with the same coordinates.
\param a Real.
*/
inline Vector::Vector(const double& a) :c{ a,a,a }
{
}

/*!
\brief Create a vector with argument coordinates.
\param a,b,c Coordinates.
*/
inline Vector::Vector(const double& a, const double& b, const double& c) :c{ a,b,c }
{
}

//! Gets the i-th coordinate of vector.
inline constexpr double& Vector::operator[] (int i)
{
  return c[i];
}

//! Returns the i-th coordinate of vector.
inline constexpr double Vector::operator[] (int i) const
{
  return c[i];
}

// // Unary operators

//! Overloaded.
inline Vector Vector::operator- () const
{
  return Vector(-c[0], -c[1], -c[2]);
}

// Assignment unary operators

//! Destructive addition.
inline Vector& Vector::operator+= (const Vector& u)
{
  c[0] += u.c[0]; c[1] += u.c[1]; c[2] += u.c[2];
  return *this;
}

//! Destructive scalar multiply.
inline Vector& Vector::operator*= (const double& a)
{
  c[0] *= a; c[1] *= a; c[2] *= a;
  return *this;
}

//! Destructive division by a scalar.
inline Vector& Vector::operator/= (const double& a)
{
  c[0] /= a; c[1] /= a; c[2] /= a;
  return *this;
}

//! Adds up two vectors.
inline Vector operator+ (const Vector& u, const Vector& v)
{
  return Vector(u.c[0] + v.c[0], u.c[1] + v.c[1], u.c[2] + v.c[2]);
}

//! Difference between two vectors.
inline Vector operator- (const Vector& u, const Vector& v)
{
  return Vector(u.c[0] - v.c[0], u.c[1] - v.c[1], u.c[2] - v.c[2]);
}

//! Right multiply by a scalar.
inline Vector operator* (const Vector& u, double a)
{
  return Vector(u.c[0] * a, u.c[1] * a, u.c[2] * a);
}

//! Left multiply by a scalar.
inline Vector operator* (const double& a, const Vector& v)
{
  return v * a;
}

//! Cross product.
inline Vector operator/ (const Vector& u, const Vector& v)
{
  return Vector(u.c[1] * v.c[2] - u.c[2] * v.c[1], u.c[2] * v.c[0] - u.c[0] * v.c[2], u.c[0] * v.c[1] - u.c[1] * v.c[0]);
}

//! Left multiply by a scalar
inline Vector operator/ (const Vector& u, double a)
{
  return Vector(u.c[0] / a, u.c[1] / a, u.c[2] / a);
}

// // Boolean functions

/*!
\brief Compute the Euclidean norm of a vector.

This function involves a square root computation, it is in general more efficient to rely on
the squared norm of a vector instead.
\param u %Vector.
\sa SquaredNorm
*/
inline double Norm(const Vector& u)
{
  return sqrt(u.c[0] * u.c[0] + u.c[1] * u.c[1] + u.c[2] * u.c[2]);
}

/*!
\brief Return a normalized vector.

Compute the inverse of its norm and scale the components.

This function does not check if the vector is null.
\param u %Vector.
*/
inline Vector Normalized(const Vector& u)
{
  return u * (1.0 / Norm(u));
}

/*!
\brief Compute the maximum component of a vector.

Note that this function is not the same as NormInfinity which computes
the maximum of the absolute values of components. The codes are equivalent:
\code
Vector a(-1.0,-3.0,2.0);
double s=NormInfinity(a);
double t=Max(Abs(a));
\endcode

\sa NormInfinity
*/
inline double Vector::Max() const
{
  return Math::Max(c[0], c[1], c[2]);
}

// Class
class Vector2
{
protected:
  double c[2] = { 0.0, 0.0 }; //!< Components.
public:
  //! Empty.
  Vector2() {}

  explicit Vector2(const double&);
  explicit Vector2(const double&, const double&);
  Vector2(const Vector&);

  double& operator[] (int);
  constexpr double operator[] (int) const;

  Vector2 Orthogonal() const;

  Vector2 operator- () const;

  // Assignment operators
  Vector2& operator+= (const Vector2&);
  Vector2& operator*= (const Vector2&);
  Vector2& operator*= (double);
  Vector2& operator/= (double);

  // Binary operators
  friend Vector2 operator+ (const Vector2&, const Vector2&);
  friend Vector2 operator- (const Vector2&, const Vector2&);

  friend double operator* (const Vector2&, const Vector2&);

  friend Vector2 operator* (const Vector2&, double);
  friend Vector2 operator* (double, const Vector2&);
  friend Vector2 operator/ (const Vector2&, double);

  friend double operator/ (const Vector2&, const Vector2&);

  // Norm
  friend double Norm(const Vector2&);
  friend double SquaredNorm(const Vector2&);
  friend double NormInfinity(const Vector2&);

   friend Vector2 Normalized(const Vector2&);

  // Conversion
  Vector ToVector(const double& = 0.0) const;

  Vector2 Inverse() const;

  // Compare functions
  static Vector2 Min(const Vector2&, const Vector2&);
  static Vector2 Max(const Vector2&, const Vector2&);

  // Modulo
  static Vector2 Mod(const Vector2&, const Vector2&);

  // Swap
  friend void Swap(Vector2&, Vector2&);

  static Vector2 Lerp(const Vector2&, const Vector2&, const double&);

  Vector2 Scaled(const Vector2&) const;

  static Vector2 Polar(const double&);

  static Vector2 Solve(const Vector2&, const Vector2&, const double&, const double&);

public:
  static const Vector2 Null; //!< Null vector.
  static const Vector2 X; //!< Vector2(1,0).
};

/*!
\brief Create a vector with the same real coordinates.
\param a Real.
*/
inline Vector2::Vector2(const double& a) :c{ a,a }
{
}

//! Create a vector with argument coordinates.
inline Vector2::Vector2(const double& a, const double& b) : c{ a,b }
{
}

//! Create a two dimension vector from another three dimension vector.
inline Vector2::Vector2(const Vector& v) : c{ v[0],v[1] }
{
}

//! Gets the i-th coordinate of vector.
inline double& Vector2::operator[] (int i)
{
  return c[i];
}

//! Returns the i-th coordinate of vector.
inline constexpr double Vector2::operator[] (int i) const
{
  return c[i];
}

/*!
\brief Convert a Vector2 to a Vector.
\param z Extra coordinate.
*/
inline Vector Vector2::ToVector(const double& z) const
{
  return Vector(c[0], c[1], z);
}

// // Unary operators

//! Overloaded.
inline Vector2 Vector2::operator- () const
{
  return Vector2(-c[0], -c[1]);
}

// // Assignment unary operators

//! Destructive addition.
inline Vector2& Vector2::operator+= (const Vector2& u)
{
  c[0] += u.c[0]; c[1] += u.c[1];
  return *this;
}

//! Destructive scalar multiply.
inline Vector2& Vector2::operator*= (double a)
{
  c[0] *= a; c[1] *= a;
  return *this;
}

//! Destructive division by a scalar.
inline Vector2& Vector2::operator/= (double a)
{
  c[0] /= a; c[1] /= a;
  return *this;
}

//! Destructively scale a vector by another vector.
inline Vector2& Vector2::operator*= (const Vector2& u)
{
  c[0] *= u.c[0]; c[1] *= u.c[1];
  return *this;
}

//! Adds up two vectors.
inline Vector2 operator+ (const Vector2& u, const Vector2& v)
{
  return Vector2(u.c[0] + v.c[0], u.c[1] + v.c[1]);
}

//! Difference between two vectors.
inline Vector2 operator- (const Vector2& u, const Vector2& v)
{
  return Vector2(u.c[0] - v.c[0], u.c[1] - v.c[1]);
}

/*!
\brief Dot product between two vectors.
\param u,v Argument vectors.
*/
inline double operator* (const Vector2& u, const Vector2& v)
{
  return u.c[0] * v.c[0] + u.c[1] * v.c[1];
}

//! Right multiply by a scalar.
inline Vector2 operator* (const Vector2& u, double a)
{
  return Vector2(u.c[0] * a, u.c[1] * a);
}

//! Left multiply by a scalar.
inline Vector2 operator* (double a, const Vector2& v)
{
  return v * a;
}

/*!
\brief Cross productof two vectors.
Note that the derminant of a 2-square matrix is the cross product of its two colum vectors.
*/
inline double operator/ (const Vector2& u, const Vector2& v)
{
  return u.c[0] * v.c[1] - u.c[1] * v.c[0];
}

//! Left divide by a scalar
inline Vector2 operator/ (const Vector2& u, double a)
{
  return Vector2(u.c[0] / a, u.c[1] / a);
}

// // Boolean functions

/*!
\brief Compute the Euclidean norm of a vector.

This function involves a square root computation, it is often more efficient to rely on
the squared norm of a vector instead. \sa SquaredNorm
*/
inline double Norm(const Vector2& u)
{
  return sqrt(u.c[0] * u.c[0] + u.c[1] * u.c[1]);
}

/*!
\brief Compute the squared Euclidean norm of a vector.
\sa Norm
*/
inline double SquaredNorm(const Vector2& u)
{
  return (u.c[0] * u.c[0] + u.c[1] * u.c[1]);
}

/*!
\brief Return a Normalized a vector, computing the inverse of its norm and scaling the components.

This function does not check if
the vector is null, which might result in errors.
*/
inline Vector2 Normalized(const Vector2& u)
{
  return u * (1.0 / Norm(u));
}

/*!
\brief Compute the infinity norm of a vector.
\sa Norm, SquaredNorm
*/
inline double NormInfinity(const Vector2& u)
{
  return Math::Max(fabs(u.c[0]), fabs(u.c[1]));
}

/*!
\brief Return a vector with coordinates set to the minimum coordinates
of the two argument vectors.
*/
inline Vector2 Vector2::Min(const Vector2& a, const Vector2& b)
{
  return Vector2(a[0] < b[0] ? a[0] : b[0], a[1] < b[1] ? a[1] : b[1]);
}

/*!
\brief Return a vector with coordinates set to the maximum coordinates
of the two argument vectors.
*/
inline Vector2 Vector2::Max(const Vector2& a, const Vector2& b)
{
  return Vector2(a[0] > b[0] ? a[0] : b[0], a[1] > b[1] ? a[1] : b[1]);
}

/*!
\brief Creates a vector given polar coordinates.
\param t Theta.
*/
inline Vector2 Vector2::Polar(const double& t)
{
  return Vector2(cos(t), sin(t));
}

/*!
\brief Scales the vector.
\param a Scaling vector.
*/
inline Vector2 Vector2::Scaled(const Vector2& a) const
{
  return Vector2(c[0] * a[0], c[1] * a[1]);
}

/*!
\brief Modulo of two vectors.
\param a,b Two vectors.
*/
inline Vector2 Vector2::Mod(const Vector2& a, const Vector2& b)
{
  return Vector2(Math::Mod(a[0], b[0]), Math::Mod(a[1], b[1]));
}

/*!
\brief Returns a direct orthogonal vector.
*/
inline Vector2 Vector2::Orthogonal() const
{
  return Vector2(-c[1], c[0]);
}

/*!
\brief %Linear interpolation between two vectors.
\param a,b Interpolated points.
\param t Interpolant.
*/
inline Vector2 Vector2::Lerp(const Vector2& a, const Vector2& b, const double& t)
{
  return a + t * (b - a);
}

/*!
\brief Compute the point on a segment such that the linear function satisfies f(a)=va and f(b)=vb.

\sa Vector::Solve(const double& a, const double& b, const Vector&, const Vector&);
*/
inline Vector2 Vector2::Solve(const Vector2& a, const Vector2& b, const double& va, const double& vb)
{
  return (vb * a - va * b) / (vb - va);
}

/*!
\brief Inverse of a vector.

This function inverses the components of the vector.
\sa Vector::Inverse()
*/
inline Vector2 Vector2::Inverse() const
{
  return Vector2(1.0 / c[0], 1.0 / c[1]);
}

