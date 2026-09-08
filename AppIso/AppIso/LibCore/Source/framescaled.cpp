// Frames 

#include "frame.h"

const FrameScaled FrameScaled::Id(Matrix(Vector(1.0)));

/*!
\brief Creates a frame.
\param r Rotation matrix.
\param t Translation vector.
\param s Scaling vector.
*/
FrameScaled::FrameScaled(const Matrix& r, const Vector& t, const Vector& s) :Frame(r, t), s(s)
{
}

/*!
\brief Get the homogeneous matrix out of the frame.
*/
Matrix4 FrameScaled::GetMatrix4() const
{
  return Matrix4(r * Matrix(s), t);
}

