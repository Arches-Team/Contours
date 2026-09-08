#pragma once

#include "scalarfield.h"

class FlowStruct
{
protected:
  int mask = 0; //!< Integer mask of downstream flow topology.
public:
  int i[8] = { -1 }; //!< Indexes to neighboring cells.
  QPoint q[8] = { QPoint(0,0) }; //!< Flow points.
  double h[8] = { 0.0 }; //!< Heights.
  double s[8] = { 0.0 }; //!< Slopes.
  double sn[8] = { 0.0 }; //!< Normalized slopes.
protected:
  int steepest = -1; //!< Steepest slope index, i.e., index in the arrays storing the steepest slope data.
public:
  explicit FlowStruct() {}
  ~FlowStruct() {}

  friend class HeightField;
};

class HeightField : public ScalarField2
{
public:
  HeightField() {}

  HeightField(const ScalarField2&);
  explicit HeightField(const Box2&, const QImage&, const double& = 0.0, const double& = 256.0 * 256.0 - 1.0, bool = true);
  explicit HeightField(const Box2&, int, int, const double& = 0.0);

  //! Empty
  ~HeightField() {}

  Vector Vertex(int, int) const;

  double AverageSlope(int, int) const;

  double K() const;

  // Stream area
  ScalarField2 StreamArea(const double& = 1.0) const;

  // Slope
  ScalarField2 AverageSlope() const;
  ScalarField2 Slope(bool = false) const;

  // Box
  Box GetBox() const;

  void Scale(const Vector&);

  // Flow
  int CheckFlowSlope(const QPoint&, FlowStruct&, const double& = 1.0) const;

  void ErosionDeposition(double = 1.0, double = 1.0);
  void ErosionAmpli(int = 3, double = 1.0, double = 1.0);

public:
  static const double flat; //!< Small negative epsilon value used in breaching and flow algorithms.
};

