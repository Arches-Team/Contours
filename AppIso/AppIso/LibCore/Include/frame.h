// Frames 

#pragma once

#include "matrix.h"
#include "evector.h"

// Frame class
class Frame
{
protected:
  Matrix r = Matrix::Identity;  //!< Rotation matrix.
  Vector t = Vector::Null;  //!< Translation vector.
public:
  Frame(const Matrix & = Matrix::Identity, const Vector & = Vector::Null);

  //! Empty
  ~Frame() {}

};

// Frame class for instances
class FrameScaled : protected Frame
{
protected:
  Vector s; //!< Scaling vector
public:
  explicit FrameScaled(const Matrix & = Matrix::Identity, const Vector & = Vector::Null, const Vector & = Vector(1.0));

  //! Empty
  ~FrameScaled() {}

  // Compute matrix
  Matrix4 GetMatrix4() const;

public:
  static const FrameScaled Id; //!< Identity.
};

