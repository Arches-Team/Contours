// Frames 

#include "frame.h"

/*!
\brief Creates a frame given a rotation matrix and a translation vector.
\param r Frame matrix.
\param t Translation vector.
*/
Frame::Frame(const Matrix& r, const Vector& t) :r(r), t(t)
{
}

