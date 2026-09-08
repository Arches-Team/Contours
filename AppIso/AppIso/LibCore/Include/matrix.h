// Matrix 

#pragma once

#include "evector.h"

class Matrix2
{
protected:
  double r[4] = { 1.0,0.0,0.0,1.0 };  //!< The array storing the coefficients of the matrix.
public:
  //! Empty.
  Matrix2() {}
  explicit Matrix2(const double&, const double&, const double&, const double&);

  Vector2 operator*(const Vector2&) const;

  static Matrix2 Rotation(const double&);

};

/*!
\brief Creates a matrix with a set of double values.
*/
inline Matrix2::Matrix2(const double& a00, const double& a01, const double& a10, const double& a11)
{
  r[0] = a00;
  r[1] = a01;

  r[2] = a10;
  r[3] = a11;
}

/*!
\brief Right multiply by a vector.
*/
inline Vector2 Matrix2::operator*(const Vector2& v) const
{
  return Vector2(v[0] * r[0] + v[1] * r[2], v[0] * r[1] + v[1] * r[3]);
}

class Matrix
{
protected:
  double r[9] = { 1.0,0.0,0.0,0.0,1.0,0.0,0.0,0.0,1.0 };  //!< The array storing the coefficients of the matrix.
public:
  //! Empty.
  Matrix() {}
  explicit Matrix(const double&);
  explicit Matrix(const Vector&);
  explicit Matrix(const double&, const double&, const double&, const double&, const double&, const double&, const double&, const double&, const double&);

  static const Matrix Identity;

  // Functions to access Matrix class components  
  constexpr double& operator[] (int);
  constexpr double operator[] (int) const;

  friend Matrix operator* (const Matrix&, const Matrix&);

};

//! Direct access to the array of the matrix.
inline constexpr double& Matrix::operator[] (int i)
{
  return r[i];
}

//! Overloaded.
inline constexpr double Matrix::operator[] (int i) const
{
  return r[i];
}

/*!
\brief Creates a matrix with a set of double values.

Coefficients are given in column order.
*/
inline Matrix::Matrix(const double& a00, const double& a01, const double& a02, const double& a10, const double& a11, const double& a12, const double& a20, const double& a21, const double& a22)
{
  r[0] = a00;
  r[1] = a01;
  r[2] = a02;

  r[3] = a10;
  r[4] = a11;
  r[5] = a12;

  r[6] = a20;
  r[7] = a21;
  r[8] = a22;
}

/*!
\brief Creates a diagonal matrix.
\param a The diagonal value.
*/
inline Matrix::Matrix(const double& a)
{
  r[1] = r[2] = r[3] = r[5] = r[6] = r[7] = 0.0;
  r[0] = r[4] = r[8] = a;
}

/*!
\brief Create a diagonal matrix with diagonal terms set to the vector entries.
\param a %Vector of diagonal values.
*/
inline Matrix::Matrix(const Vector& a)
{
  r[1] = r[2] = r[3] = r[5] = r[6] = r[7] = 0.0;
  r[0] = a[0];
  r[4] = a[1];
  r[8] = a[2];
}

// Extended matrix
class Matrix4
{
protected:
  double r[16] = { 1.0,0.0,0.0,0.0,0.0,1.0,0.0,0.0,0.0,0.0,1.0,0.0,0.0,0.0,0.0,1.0 }; //!< Coefficients.
public:
  //! Empty
  Matrix4() {}
  explicit Matrix4(const Matrix&, const Vector&);
  explicit Matrix4(const Matrix&, const Vector&, const Vector&);

  // Float
  void Float(float[16]) const;

};

