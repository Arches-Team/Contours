// Fundamentals
#pragma once

#include "array.h"
#include "histogram.h"
#include "palette.h"
#include "segment.h"

class ScalarField2;

class AnalyticScalarField2
{
protected:
  bool sign = true; //!< Sign convention, used for normal computation.
public:
  AnalyticScalarField2(bool = true);
  virtual double Value(const Vector2&) const;
  virtual ScalarField2 Sample(const Array2&) const;

};

/*!
\brief Constructor.

\param s Sign, by default set sign convention to true, i.e., negative values inside and positive outside.
*/
inline AnalyticScalarField2::AnalyticScalarField2(bool s) :sign(s)
{
}

/*!
\class ScalarPoint2
\brief Internal class for sorting points.
*/
class ScalarPoint2
{
protected:
  QPoint p; //!< Point.
  double z; //!< Elevation
public:
  //! Empty.
  ScalarPoint2() :p(0, 0), z(0.0) {}
  explicit ScalarPoint2(const QPoint&, const double&);
  friend bool operator<(const ScalarPoint2& a, const ScalarPoint2& b) { return a.z < b.z; }

  // Access to members
  QPoint Point() const;
};

/*!
\brief Create a scalar point.
\param p Point.
\param s Elevation.
*/
inline ScalarPoint2::ScalarPoint2(const QPoint& p, const double& s) :p(p), z(s)
{
}

/*!
\brief Get point.
*/
inline QPoint ScalarPoint2::Point() const
{
  return p;
}

class FloatArray {
protected:
  QVector<float> a; //!< %Array.
public:
  FloatArray() {}
  FloatArray(int);
  float& operator[](int);
};

inline FloatArray::FloatArray(int n)
{
  a.resize(n);
}

/*!
\brief Access to element.
*/
inline float& FloatArray::operator[](int i)
{
  return a[i];
}

class ScalarField2 : public Array2
{
protected:
  QVector<double> field; //!< Field samples.
public:
  ScalarField2() {  }
  ScalarField2(const Array2&, const double& = 0.0);
  explicit ScalarField2(const Box2&, int, int, const double& = 0.0);
  explicit ScalarField2(const Box2&, const QImage&, const double& = 0.0, const double& = 256.0 * 256.0 - 1.0, bool = true);
  explicit ScalarField2(const QImage&, const double& = 1.0, const double& = 1.0, bool = true);

  void GetRange(double&, double&) const;

  void Symmetry(bool = true, bool = false);

  Histogram GetHistogram(int) const;

  virtual Vector2 Gradient(int, int) const;
  virtual double K() const;

  // Access to elements
  double at(int, int) const;
  double at(const QPoint&) const;
  double& operator()(int, int);
  double& operator()(const QPoint&);

  double at(int) const;
  double& operator[](int);

  // Create image from the scalar field
  QImage CreateImage(bool = true) const;

  QImage CreateImage(const double&, const double&, const GenericPalette&, bool = false) const;
  QImage CreateImage(const GenericPalette&) const;
  QImage CreateImage(const double&, const double&, bool = true) const;

  ScalarField2 Resized(int, int, bool = false) const;

  ScalarField2 DownSample(int) const;

  virtual double Value(const Vector2&) const;
  virtual double BiCubicValue(const Vector2&) const;
  virtual double Closest(const Vector2&) const;

  // Smoothing
  void Smooth();
  void Smooth(int);

  void Scale(const Vector2&);
  void Scale(const double&);

  // Functions
  void Normalize();

  ScalarField2& operator*=(const double&);

  ScalarField2& operator-=(const ScalarField2&);

  ScalarField2& operator+=(const ScalarField2&);

  friend ScalarField2 operator-(const ScalarField2&, const ScalarField2&);

  friend ScalarField2 operator+(const ScalarField2&, const ScalarField2&);

  friend ScalarField2 operator*(const double&, const ScalarField2&);

  void Pow(const double&);
  void SetRange(const double&, const double&);

  ScalarField2 GradientNorm() const;

  ScalarField2 Ln() const;

  QVector<ScalarPoint2> GetScalarPoints() const;
  FloatArray GetAsFloats() const;

  Vector2 Dichotomy(Vector2, Vector2, double, double, double, const double&, const double&) const;

  SegmentSet2 LineSegments(const double&, bool = false) const;

public:
  // Provide range-based for loops
  auto begin() { return field.begin(); }
  auto end() { return field.end(); }
  auto cbegin() const { return field.begin(); }
  auto cend() const { return field.end(); }
  auto begin() const { return field.begin(); }
  auto end() const { return field.end(); }

};

/*!
\brief Return the field value at a given array vertex.
\param i,j Integer coordinates of the vertex.
*/
inline double ScalarField2::at(int i, int j) const
{
  return field.at(VertexIndex(i, j));
}

/*!
\brief Return the field value at a given array vertex.
\param q Point.
*/
inline double ScalarField2::at(const QPoint& q) const
{
  return field.at(VertexIndex(q.x(), q.y()));
}

/*!
\brief Return the field value at a given array vertex.
\param q Point.
*/
inline double& ScalarField2::operator()(const QPoint& q)
{
  return field[VertexIndex(q.x(), q.y())];
}

/*!
\brief Return the field value at a given array vertex.
\param i,j Integer coordinates of the vertex.
*/
inline double& ScalarField2::operator()(int i, int j)
{
  return field[VertexIndex(i, j)];
}

/*!
\brief Return the data in the field.
\param c Index.
*/
inline double ScalarField2::at(int c) const
{
  return field.at(c);
}

/*!
\brief Return the data in the field.
\param c Index.
*/
inline double& ScalarField2::operator[](int c)
{
  return field[c];
}

/*!
\brief Subtraction.

\param a,b Scalar fields.
*/
inline ScalarField2 operator-(const ScalarField2& a, const ScalarField2& b)
{
  ScalarField2 r = a;
  r -= b;
  return r;
}

/*!
\brief Addition.

\param a,b Scalar fields.
*/
inline ScalarField2 operator+(const ScalarField2& a, const ScalarField2& b)
{
  ScalarField2 r = a;
  r += b;
  return r;
}

/*!
\brief Multiplication.

\param a Scalar field.
\param x Real.
*/
inline ScalarField2 operator*(const double& x, const ScalarField2& a)
{
  ScalarField2 r = a;
  r *= x;
  return r;
}

