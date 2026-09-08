// Camera

#include "camera.h"

/*!
\brief Create a default camera.
*/
Camera::Camera() :Camera(Vector::Null, Vector::Y, Vector::Z, 1.0, 1.0, 1.0, 1000.0)
{
}

/*!
\brief Create a camera given its location and look-at point.

If no upward vector is provided, it is defined as z-axis.

\image html camera.png

The view vector is defined as the vector between the eye point and the look at point.
The right vector, which is always computed as a cross product between the view vector and the up vector.
\param eye Eye point.
\param at Look-at point.
\param up Up vector.
\param width, height Width and height of virtual screen.
\param near, far Near and far planes.
*/
Camera::Camera(const Vector& eye, const Vector& at, const Vector& up, const double& width, const double& height, const double& near, const double& far) :eye(eye), at(at), up(up), width(width), height(height), nearplane(near), farplane(far)
{
  view = Normalized(at - eye);

  // Aperture
  Camera::cah = 0.980;
  Camera::cav = 0.735;
}

/*!
\brief Moves the eye point towards or away from the look at point.

The look-at point does not change.

\param a Distance.
\param t Boolean, set to true if look-at point should also be moved in the direction.
*/
void Camera::BackForth(const double& a, bool t)
{
  eye += a * view;
  if (t == true)
  {
    at += a * view;
    // View unchanged
  }
  else
  {
    view = Normalized(at - eye);
  }
}

/*!
\brief Rotates the camera relatively to the look-at point.
\param a Distance.
*/
void Camera::LeftRightRound(const double& a)
{
  Vector e = eye - at;
  Vector left = up / e;
  e = Vector(e[0] * cos(a) - e[1] * sin(a), e[0] * sin(a) + e[1] * cos(a), e[2]);
  left = Vector(left[0] * cos(a) - left[1] * sin(a), left[0] * sin(a) + left[1] * cos(a), 0.0);
  up = Normalized(left / -e);
  eye = at + e;
  // View
  view = Normalized(at - eye);
}

/*!
\brief Rotates the camera relatively to the look-at point.
\param a Distance.
*/
void Camera::UpDownRound(const double& a)
{
  Vector z = at - eye;
  double length = Norm(z);
  z /= length;
  Vector left = up / z;
  left /= Norm(left);

  // Rotate
  z = z * cos(a) + up * sin(a);

  // Update Vector
  up = z / left;
  eye = at - z * length;
  // View
  view = Normalized(at - eye);
}

/*!
\brief Moves the camera left or right, preserving its height.
\param a Distance.
*/
void Camera::LeftRightHorizontal(const double& a)
{
  Vector z = at - eye;
  z[2] = 0.0;
  double length = Norm(z);
  z /= length;
  Vector left = Vector::Z / z;
  left /= Norm(left);

  eye += a * left;
  at += a * left;
  // View unchanged
}

/*!
\brief Moves the camera vertically.

This function keeps the left vector horizontal.
\param a Distance.
*/
void Camera::UpDownVertical(const double& a)
{
  Vector left = Vector::Z / view;
  left /= Norm(left);

  eye += a * view / left;
  at += a * view / left;
  // View unchanged
}

/*!
\brief Returns the horizontal angle of view.

Angle is in radian.
*/
double Camera::GetAngleOfViewH() const
{
  // http://www.scratchapixel.com/lessons/3d-advanced-lessons/cameras-advanced-techniques/film-aperture-focal-length/

  // Horizontal angle of view in degrees 
  return 2.0 * atan(cah * 25.4 * 0.5 / fl);
}

/*!
\brief Returns the vertical angle of view.

Angle is in radian.

\param w, h Width and height of the screen.
*/
double Camera::GetAngleOfViewV(double w, double h) const
{
  // Horizontal angle of view  
  double avh = GetAngleOfViewH();

  double avv = 2.0 * atan(tan(avh / 2.0) * double(h) / double(w));

  // Vertical angle of view
  return avv;
}

/*!
\brief Create a QString form a camera.
\param n Number of digits used in the representation of reals.
*/
QString Camera::ToString(int n) const
{
  return QString("Eye(%1,%2,%3)\n At(%4,%5,%6)").arg(eye[0], 0, 'f', n).arg(eye[1], 0, 'f', n).arg(eye[2], 0, 'f', n).arg(at[0], 0, 'f', n).arg(at[1], 0, 'f', n).arg(at[2], 0, 'f', n);
}

/*!
\brief Compute the equation of a ray given a pixel in the camera plane.
\param px,py Pixel coordinates.
\param w,h Size of the viewing window.
*/
Ray Camera::PixelToRay(int px, int py, int w, int h) const
{
  // Get coordinates
  Vector horizontal = Normalized(view / Up());
  Vector vertical = Normalized(horizontal / view);

  double length = 1.0;

  // Convert to radians 
  double rad = GetAngleOfViewV(w, h);  // fov

  double vLength = tan(rad / 2.0) * length;
  double hLength = vLength * (double(w) / double(h));

  vertical *= vLength;
  horizontal *= hLength;

  // Translate mouse coordinates so that the origin lies in the center of the view port
  double x = px - w / 2.0;
  double y = h / 2.0 - py;

  // Scale mouse coordinates so that half the view port width and height becomes 1.0
  x /= w / 2.0;
  y /= h / 2.0;

  // Direction is a linear combination to compute intersection of picking ray with view port plane
  return Ray(eye, Normalized(view * length + horizontal * x + vertical * y));
}

/*!
\brief Create a default camera focusing on a given box.

THe camera is located outside of the box and focuses on the center of the target box.

\param box The box.
*/
Camera Camera::View(const Box& box)
{
  // Planar rectangle
  Box2 r = Box2(box);

  // Viewing diagonal
  Vector2 v = 0.5 * r.Diagonal();

  return Camera(box.Center() - (2.0 * v).ToVector(-Norm(v)), box.Center(), Vector(0.0, 0.0, 1.0), 1.0, 1.0, 5.0, 10000);
}

/*!
\brief Sets the camera target vector.
\param a Look-at point.
*/
void Camera::SetAt(const Vector& a)
{
  at = a;
  up = Vector::Z;
  // View
  view = Normalized(at - eye);
}

/*!
\brief Sets the camera eye point.
\param p Eye point.
*/
void Camera::SetEye(const Vector& p)
{
  eye = p;
  // View
  view = Normalized(at - eye);
}

/*!
\brief Set the near and far planes.
\param n, f Near and far planes distance to th eye.
*/
void Camera::SetPlanes(const double& n, const double& f)
{
  nearplane = n;
  farplane = f;
}

