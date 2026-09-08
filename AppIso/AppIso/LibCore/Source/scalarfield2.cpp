
#include "scalarfield.h"

#include "cubic.h"
#include "linear.h"
#include "cpu.h"

/*!
\class ScalarField2 scalarfield.h
\brief A base two-dimensional field of real values.

\ingroup StructureGroup
*/

/*!
\brief Create the field structure.
\param a Array representing the grid domain.
\param v Constant value of field.
*/
ScalarField2::ScalarField2(const Array2& a, const double& v) : Array2(a)
{
  field.fill(v, nx * ny);
}

/*!
\brief Create the field structure.
\param box The box.
\param x,y Size of the array.
\param v Constant value of field.
*/
ScalarField2::ScalarField2(const Box2& box, int x, int y, const double& v) : ScalarField2(Array2(box, x, y), v)
{
}

/*!
\brief Create a field structure from an image.
\param image The image.
\param v Range of values to scale the image, values will vary between 0.0 and v.
\param d Distance between samples.
\param grayscale Read image as grayscale if set to true, otherwize use color for a better accuracy.
*/
ScalarField2::ScalarField2(const QImage& image, const double& v, const double& d, bool grayscale) : ScalarField2(Box2(image.size()).Scaled(d), image, 0.0, v, grayscale)
{
}

/*!
\brief Create a field structure from an image.
\param box The box.
\param image The image.
\param a,b Range of values to scale the image.
\param grayscale Read image as grayscale if set to true, otherwize use color for a better accuracy.
*/
ScalarField2::ScalarField2(const Box2& box, const QImage& image, const double& a, const double& b, bool grayscale) : Array2(box, image.width(), image.height())
{
  // Set size
  field.resize(nx * ny);

  // Write Heightmap
  for (int i = 0; i < image.width(); i++)
  {
    for (int j = 0; j < image.height(); j++)
    {
      double t = 0.0;

      // Grayscale
      if (grayscale)
      {
        // Grayscale 16 bits
        if (image.format() == QImage::Format_Grayscale16 || image.format() == QImage::Format_RGBA64)
        {
          QColor thecolor = image.pixelColor(i, j);
          t = thecolor.blueF();
        }
        // Grayscale 8 bits
        else
        {
          QRgb color = image.pixel(i, j);
          t = double(qGray(color)) / 255.0;
        }
      }
      // Color 
      else
      {
        QRgb color = image.pixel(i, j);
        // Maximum value is 256^3-1
        int u = qRed(color) << 16 | qGreen(color) << 8 | qBlue(color);

        t = double(u) / (16777216.0 - 1.0);
      }
      field[VertexIndex(i, j)] = Math::Lerp(a, b, t);
    }
  }
}

//     // Could try this instead of the previous four lines
//     // avxa = _mm256_hadd_pd(avxa, avxa);
//     // s= ((double*)&avxa)[0] + ((double*)&avxa)[2];

/*!
\brief Perform a symmetry.
\param x, y Axes of symmetry.
*/
void ScalarField2::Symmetry(bool x, bool y)
{
  if ((x == false) && (y == false)) return;

  else if ((x == false) && (y == true))
  {
    for (int i = 0; i < nx; i++)
    {
      for (int j = 0; j < ny / 2; j++)
      {
        Math::Swap(field[VertexIndex(i, j)], field[VertexIndex(i, ny - 1 - j)]);
      }
    }
  }
  else if ((x == true) && (y == false))
  {
    for (int i = 0; i < nx / 2; i++)
    {
      for (int j = 0; j < ny; j++)
      {
        Math::Swap(field[VertexIndex(i, j)], field[VertexIndex(nx - 1 - i, j)]);
      }
    }
  }
  else
  {
    Symmetry(true, false);
    Symmetry(false, true);
  }
}

/*!
\brief Get the range of the field.

\avx

\param a,b Returned minimum and maximum.
*/
void ScalarField2::GetRange(double& a, double& b) const
{
  const int size = nx * ny;
  // Escape
  if (size == 0)
  {
    a = b = 0.0;
    return;
  }

#ifdef _MSC_VER
  if (System::Avx() && field.size() > 8)
  {
    const double* p = field.data();

    __m256d avxa = _mm256_load_pd(p);
    __m256d avxb = avxa;

    const int offset = size % 4;
    for (int i = 1; i < size / 4; i++)
    {
      __m256d avxi = _mm256_load_pd(p + i * 4);
      avxa = _mm256_min_pd(avxa, avxi);
      avxb = _mm256_max_pd(avxb, avxi);
    }
    double e[4];
    _mm256_store_pd(e, avxa);
    a = Math::Min(e[0], e[1], e[2], e[3]);

    _mm256_store_pd(e, avxb);
    b = Math::Max(e[0], e[1], e[2], e[3]);

    for (int i = size - offset; i < size; i++)
    {
      double x = field.at(i);
      if (x < a)
      {
        a = x;
      }
      else if (x > b)
      {
        b = x;
      }
    }
  }
  else
#endif
  {
    a = field.at(0);
    b = a;

    for (int i = 1; i < field.size(); i++)
    {
      double x = field.at(i);
      if (x < a)
      {
        a = x;
      }
      else if (x > b)
      {
        b = x;
      }
    }
  }
}

/*!
\brief Compute the histogram of the scalar field.
\param n Discretization.
*/
Histogram ScalarField2::GetHistogram(int n) const
{
  return Histogram(n, field);
}

/*!
\brief Create an image from the field.
\param grayscale Export as grayscale if set to true, and to color otherwise (provides a better precision).
*/
QImage ScalarField2::CreateImage(bool grayscale) const
{
  double a, b;
  GetRange(a, b);

  // Check if the scalarfield is constant, in that case modify the range to avoid division by 0 errors
  if (a == b)
  {
    b = a + 1.0;
  }
  return CreateImage(a, b, grayscale);
}

/*!
\brief Create an image from the field using a palette.
\param palette The palette.
*/
QImage ScalarField2::CreateImage(const GenericPalette& palette) const
{
  double a, b;
  GetRange(a, b);
  return CreateImage(a, b, palette);
}

/*!
\brief Create an image from the field using a palette.
\param a,b Range of elevation that will be mapped to image scale.
\param palette The palette.
\param transparent If set to true, the lowest values of the scalar field will be transparent.
*/
QImage ScalarField2::CreateImage(const double& a, const double& b, const GenericPalette& palette, bool transparent) const
{
  // Define image
  QImage image(nx, ny, QImage::Format_ARGB32);

  // Write Heightmap
  for (int i = 0; i < image.width(); i++)
  {
    for (int j = 0; j < image.height(); j++)
    {
      double x = field.at(VertexIndex(i, j));
      double y = Linear::Step(x, a, b);

      QColor color = palette.GetColor(y).GetQt();

      // Transparency
      if (transparent)
      {
        if (y < 0.01)
        {
          color.setAlpha(0);
        }
      }
      image.setPixel(i, j, color.rgba());
    }
  }
  return image;
}

/*!
\brief Create an image from the field.
\param a,b Range of elevation that will be mapped to image scale.
\param grayscale Export as grayscale if set to true, color otherwise.
*/
QImage ScalarField2::CreateImage(const double& a, const double& b, bool grayscale) const
{
  // Write Heightmap
  if (grayscale)
  {
    QImage image(nx, ny, QImage::Format_Grayscale16);
    for (int j = 0; j < image.height(); j++)
    {
      quint16* dst = reinterpret_cast<quint16*>(image.bits() + j * image.bytesPerLine());
      for (int i = 0; i < image.width(); i++)
      {
        double x = field.at(VertexIndex(i, j));
        double y = Linear::Step(x, a, b);

        unsigned short pixelval = ushort(y * 65535.0);

        dst[i] = pixelval;
      }
    }
    return image;
  }
  else
  {
    // Define image
    QImage image(nx, ny, QImage::Format_ARGB32);

    // Write Heightmap
    for (int i = 0; i < image.width(); i++)
    {
      for (int j = 0; j < image.height(); j++)
      {
        double x = field.at(VertexIndex(i, j));
        double y = Linear::Step(x, a, b);

        QColor color;

        int c = int(y * (256.0 * 256.0 * 256.0 - 1.0));
        int cr = (c >> 16) & 255;
        int cv = (c >> 8) & 255;
        int cb = c & 255;

        color = QColor(cr, cv, cb);

        image.setPixel(i, j, color.rgb());
      }
    }
    return image;
  }

}

/*!
\brief Get the field value at a given point.

This function computes a bi-linear interpolation of the values.

\sa Math::Bilinear
\param p Point (should be strictly inside the box domain).
*/
double ScalarField2::Value(const Vector2& p) const
{
  double u, v;
  int i, j;
  CellInteger(p, i, j, u, v);

  // Test position
  if (!InsideCellIndex(i, j))
  {
    return 0.0;
  }

  return Math::Bilinear(at(i, j), at(i + 1, j), at(i + 1, j + 1), at(i, j + 1), u, v);
}

/*!
\brief Compute the value of the field at the closest grid point.
\param p Point (should be strictly inside the box domain).

\sa Value(const Vector2&),
*/
double ScalarField2::Closest(const Vector2& p) const
{
  double u, v;
  int i, j;
  CellInteger(p, i, j, u, v);

  if (!InsideCellIndex(i, j))
  {
    return 0.0;
  }

  i = int(i + u + 0.5);
  j = int(j + v + 0.5);

  return at(i, j);
}

/*!
\brief Compute the bicubic interpolation.
\author Lois Paulin.
\param p Point (should be strictly within bounding box of the domain).
*/
double ScalarField2::BiCubicValue(const Vector2& p) const
{
  double u, v;
  int i, j;
  CellInteger(p, i, j, u, v);

  double a00 = InsideVertexIndex(i, j) ? at(i, j) : 0.0;
  double a01 = InsideVertexIndex(i, j + 1) ? at(i, j + 1) : a00;
  double a10 = InsideVertexIndex(i + 1, j) ? at(i + 1, j) : a00;
  double a11 = InsideVertexIndex(i + 1, j + 1) ? at(i + 1, j + 1) : a00;

  double am10 = InsideVertexIndex(i - 1, j) ? at(i - 1, j) : a00;
  double am11 = InsideVertexIndex(i - 1, j + 1) ? at(i - 1, j + 1) : a01;

  double a0m1 = InsideVertexIndex(i, j - 1) ? at(i, j - 1) : a00;
  double a1m1 = InsideVertexIndex(i + 1, j - 1) ? at(i + 1, j - 1) : a10;

  double a02 = InsideVertexIndex(i, j + 2) ? at(i, j + 2) : a01;
  double a12 = InsideVertexIndex(i + 1, j + 2) ? at(i + 1, j + 2) : a11;

  double a20 = InsideVertexIndex(i + 2, j) ? at(i + 2, j) : a10;
  double a21 = InsideVertexIndex(i + 2, j + 1) ? at(i + 2, j + 1) : a11;

  double am1m1 = InsideVertexIndex(i - 1, j - 1) ? at(i - 1, j - 1) : am10;
  double am12 = InsideVertexIndex(i - 1, j + 2) ? at(i - 1, j + 2) : am11;
  double a2m1 = InsideVertexIndex(i + 2, j - 1) ? at(i + 2, j - 1) : a20;
  double a22 = InsideVertexIndex(i + 2, j + 2) ? at(i + 2, j + 2) : a21;

  am10 = Cubic::Interpolation(v, am1m1, am10, am11, am12);
  a00 = Cubic::Interpolation(v, a0m1, a00, a01, a02);
  a10 = Cubic::Interpolation(v, a1m1, a10, a11, a12);
  a20 = Cubic::Interpolation(v, a2m1, a20, a21, a22);

  return Cubic::Interpolation(u, am10, a00, a10, a20);
}

/*!
\brief Change the resolution of the scalar field.

Note that because the box should be strictly inside the original domain, this
function is not the same as:
\code
ScalarField field(Box2(Vector2(-2.0),Vector(3.0),5,5); // Original field
ScalarField s=Sample(field.GetBox(),12,12);
\endcode
\param x,y Sampling size.
\param bicubic Bicubic flag, set to true to use bicubic interpolation.
*/
ScalarField2 ScalarField2::Resized(int x, int y, bool bicubic) const
{
  // Sampled scalar field
  ScalarField2 sampled(Box2(a, b), x, y);

  // Corners
  sampled(0, 0) = at(0, 0);
  sampled(0, y - 1) = at(0, ny - 1);
  sampled(x - 1, 0) = at(nx - 1, 0);
  sampled(x - 1, y - 1) = at(nx - 1, ny - 1);

  if (bicubic == false)
  {
    // Borders (use linear interpolation)
    for (int i = 1; i < x - 1; i++)
    {
      double tx = (nx - 1) * (i / double(x - 1));
      int x0 = int(floor(tx));
      int x1 = int(ceil(tx));

      sampled(i, 0) = Math::Lerp(at(x0, 0), at(x1, 0), tx - x0);
      sampled(i, y - 1) = Math::Lerp(at(x0, ny - 1), at(x1, ny - 1), tx - x0);
    }

    for (int j = 1; j < y - 1; j++)
    {
      double ty = (ny - 1) * (j / double(y - 1));
      int y0 = int(floor(ty));
      int y1 = int(ceil(ty));

      sampled(0, j) = Math::Lerp(at(0, y0), at(0, y1), ty - y0);
      sampled(x - 1, j) = Math::Lerp(at(nx - 1, y0), at(nx - 1, y1), ty - y0);
    }

    // Interior
    for (int i = 1; i < x - 1; i++)
    {
      for (int j = 1; j < y - 1; j++)
      {
        sampled(i, j) = Value(sampled.ArrayVertex(i, j));
      }
    }
  }
  else
  {
    // Edges
    for (int i = 1; i < x - 1; i++)
    {
      double tx = (nx - 1) * (i / double(x - 1));
      int x0 = int(floor(tx));
      int x1 = int(ceil(tx));

      sampled(i, 0) = Math::Lerp(at(x0, 0), at(x1, 0), tx - x0);
      sampled(i, y - 1) = Math::Lerp(at(x0, ny - 1), at(x1, ny - 1), tx - x0);
    }

    for (int j = 1; j < y - 1; j++)
    {
      double ty = (ny - 1) * (j / double(y - 1));
      int y0 = int(floor(ty));
      int y1 = int(ceil(ty));

      sampled(0, j) = Math::Lerp(at(0, y0), at(0, y1), ty - y0);
      sampled(x - 1, j) = Math::Lerp(at(nx - 1, y0), at(nx - 1, y1), ty - y0);
    }

    // Interior
    for (int i = 1; i < x - 1; i++)
    {
      for (int j = 1; j < y - 1; j++)
      {
        sampled(i, j) = BiCubicValue(sampled.ArrayVertex(i, j));
      }
    }
  }
  return sampled;
}

/*!
\brief Lower the resolution of the scalarfield.
\param f Lowering factor.
*/
ScalarField2 ScalarField2::DownSample(int f) const
{
  ScalarField2 res(Box2(a, b), nx / f, ny / f);

  for (int i = 0; i < nx / f; i++)
  {
    for (int j = 0; j < ny / f; j++)
    {
      double sij = 0.0;
      for (int ti = i * f; ti < i * f + f; ti++)
      {
        for (int tj = j * f; tj < j * f + f; tj++)
        {
          sij += at(ti, tj);
        }
      }
      res(i, j) = sij / (f * f);
    }
  }

  return res;
}

/*!
\brief Compute the gradient at a given array vertex.

\param i,j Integer coordinates of the array vertex.
*/
Vector2 ScalarField2::Gradient(int i, int j) const
{
  Vector2 n;

  // Gradient along x axis
  if (i == 0)
  {
    n[0] = (at(i + 1, j) - at(i, j)) * inversecelldiagonal[0];
  }
  else if (i == nx - 1)
  {
    n[0] = (at(i, j) - at(i - 1, j)) * inversecelldiagonal[0];
  }
  else
  {
    n[0] = (at(i + 1, j) - at(i - 1, j)) * 0.5 * inversecelldiagonal[0];
  }

  // Gradient along y axis
  if (j == 0)
  {
    n[1] = (at(i, j + 1) - at(i, j)) * inversecelldiagonal[1];
  }
  else if (j == ny - 1)
  {
    n[1] = (at(i, j) - at(i, j - 1)) * inversecelldiagonal[1];
  }
  else
  {
    n[1] = (at(i, j + 1) - at(i, j - 1)) * 0.5 * inversecelldiagonal[1];
  }

  return n;
}

/*!
\brief Compute the gradient norm scalar field.
*/
ScalarField2 ScalarField2::GradientNorm() const
{
  // Scalar field of the same size
  ScalarField2 n(GetBox(), nx, ny);

  for (int i = 0; i < nx; i++)
  {
    for (int j = 0; j < ny; j++)
    {
      n(i, j) = Norm(Gradient(i, j));
    }
  }
  return n;
}

/*!
\brief Compute the logarithm of the scalar field.
*/
ScalarField2 ScalarField2::Ln() const
{
  // Scalar field of the same size
  ScalarField2 r(GetBox(), nx, ny);

  const int size = nx * ny;

  for (int i = 0; i < size; i++)
  {
    r[i] = log(at(i));
  }

  return r;
}

/*!
\brief Compute the Lipschitz constant of the elevation function.

Note that this is equivalent to the following code, with the
difference that this function does not store the norm of the gradient in memory.
\code
ScalarField2 n=f.GradientNorm();
double k=0.0;
for (int i=0;i<f.VertexSize();i++)
{
k=Math::Max(k,f.at(i));
}
\endcode
*/
double ScalarField2::K() const
{
  double k = 0.0;

  for (int i = 0; i < nx; i++)
  {
    for (int j = 0; j < ny; j++)
    {
      k = Math::Max(k, Norm(Gradient(i, j)));
    }
  }
  return k;
}

/*!
\brief Normalize the values of a scalar field to unit interval.

Compute range and apply the affine transformation to map values to [0,1].

\sa Unitize()
\avx
*/
void ScalarField2::Normalize()
{
  const int size = nx * ny;
  const int offset = size % 4;

  // Escape
  if (size == 0)
    return;

  double a, b;
  GetRange(a, b);
  if (a == b)
  {
    field.fill(1.0);
    return;
  }
#ifdef _MSC_VER
  if (System::Avx())
  {
    double* p = field.data();

    __m256d avxiba = _mm256_set1_pd(1.0 / (b - a));
    __m256d avxa = _mm256_set1_pd(a);

    for (int i = 0; i < size / 4; i++)
    {
      __m256d avx = _mm256_load_pd(p + i * 4);
      // Could use _mm256_fmadd_ps
      avx = _mm256_sub_pd(avx, avxa);
      avx = _mm256_mul_pd(avx, avxiba);
      _mm256_store_pd(p + i * 4, avx);
    }

    for (int i = size - offset; i < size; i++)
    {
      field[i] = (field[i] - a) / (b - a);
    }
  }
  else
#endif
  {
    for (int i = 0; i < field.size(); i++)
    {
      field[i] = (field[i] - a) / (b - a);
    }
  }
}

/*!
\brief Scale the values of a scalar field.

\param s Scaling factor.
*/
ScalarField2& ScalarField2::operator*=(const double& s)
{
  const int size = nx * ny;

  for (int i = 0; i < size; i++)
  {
    field[i] *= s;
  }
  return *this;
}

/*!
\brief Subtraction.
\param s Scalar field.

\avx
*/
ScalarField2& ScalarField2::operator-=(const ScalarField2& s)
{
#ifdef _MSC_VER
  if (System::Avx())
  {
    const double* sp = s.field.data();
    double* p = field.data();

    const int size = nx * ny;
    const int offset = size % 4;
    for (int i = 0; i < size / 4; i++)
    {
      __m256d avx = _mm256_load_pd(p + i * 4);
      __m256d savx = _mm256_load_pd(sp + i * 4);
      __m256d subavx = _mm256_sub_pd(avx, savx);
      _mm256_store_pd(p + i * 4, subavx);
    }

    for (int i = size - offset; i < size; i++)
    {
      field[i] -= s.field.at(i);
    }
  }
  else
#endif
  {
    int n = field.size();
    double* fieldp = field.data();
    const double* sfieldp = s.field.data();

    for (int i = 0; i < n; i++)
    {
      fieldp[i] -= sfieldp[i];
    }
  }
  return *this;
}

/*!
\brief Addition.
\param s Scalar field.
\avx
*/
ScalarField2& ScalarField2::operator+=(const ScalarField2& s)
{
#ifdef _MSC_VER
  if (System::Avx())
  {
    const double* sp = s.field.data();
    double* p = field.data();

    const int size = nx * ny;
    const int offset = size % 4;
    for (int i = 0; i < size / 4; i++)
    {
      __m256d avx = _mm256_load_pd(p + i * 4);
      __m256d savx = _mm256_load_pd(sp + i * 4);
      __m256d addavx = _mm256_add_pd(avx, savx);
      _mm256_store_pd(p + i * 4, addavx);
    }

    for (int i = size - offset; i < size; i++)
    {
      field[i] += s.field.at(i);
    }
  }
  else
#endif
  {
    int n = field.size();
    double* fieldp = field.data();
    const double* sfieldp = s.field.data();

    for (int i = 0; i < n; i++)
    {
      fieldp[i] += sfieldp[i];
    }
  }
  return *this;
}

/*!
\brief Power.
\param s Real.
*/
void ScalarField2::Pow(const double& s)
{
  for (int i = 0; i < field.size(); i++)
  {
    field[i] = pow(field[i], s);
  }
}

/*!
\brief Scales the scalar field to a given range interval.

\param a,b Interval.
*/
void ScalarField2::SetRange(const double& a, const double& b)
{
  double x, y;
  GetRange(x, y);
  if (x == y)
  {
    field.fill(a, field.size());
  }
  else
  {
    const double c = (b - a) / (y - x);
    for (int i = 0; i < field.size(); i++)
    {
      field[i] = a + c * (field.at(i) - x);
    }
  }
}

/*!
\brief Scale the domain of the scalar field.

\param s Scaling factor.
*/
void ScalarField2::Scale(const Vector2& s)
{
  Array2::Scale(s);
}

/*!
\brief Scale the scalar field.

This function also scales the values, it is different from ScalarField2::Scale(const Vector2&);

\param s Scaling factor.
*/
void ScalarField2::Scale(const double& s)
{
  Array2::Scale(s);

  const int size = nx * ny;

  for (int i = 0; i < size; i++)
  {
    field[i] *= s;
  }
}

/*!
\brief Create the set of scalar points.
*/
QVector<ScalarPoint2> ScalarField2::GetScalarPoints() const
{
  // Create array
  QVector<ScalarPoint2> e(nx * ny);

  int k = 0;
  for (int i = 0; i < nx; i++)
  {
    for (int j = 0; j < ny; j++)
    {
      e[k++] = ScalarPoint2(QPoint(i, j), at(i, j));
    }
  }
  return e;
}

/*!
\brief Compute and return an array of 4 byte floats.
*/
FloatArray ScalarField2::GetAsFloats() const
{
  FloatArray a(nx * ny);
  for (int i = 0; i < nx * ny; i++)
  {
    a[i] = at(i);
  }
  return a;
}

/*!
\brief Compute the iso-contour of the scalar field.
\param T Threshold.
\param closed Set to true if polylines should be closed at the border of the domain.
*/
SegmentSet2 ScalarField2::LineSegments(const double& T, bool closed) const
{
  // Vertices
  QVector<Vector2> v;
  v.reserve(nx * ny / 4);
  int nv = 0;

  // Edges
  QVector<int> eax(nx * ny, -1);
  QVector<int> eay(nx * ny, -1);

  // Compute straddling edges 
  for (int i = 0; i < nx - 1; i++)
  {
    for (int j = 0; j < ny; j++)
    {
      // Different signs : do not use Math::SameSign(at(i, j) - T, at(i + 1, j) - T) to preserve consistency with threshold comparison in the edge general step
      if (!((at(i, j) - T > 0.0) == (at(i + 1, j) - T > 0.0)))
      {
        v.append(Dichotomy(ArrayVertex(i, j), ArrayVertex(i + 1, j), at(i, j) - T, at(i + 1, j) - T, celldiagonal[0], T, epsilon));
        eax[VertexIndex(i, j)] = nv;
        nv++;
      }
    }
  }

  for (int i = 0; i < nx; i++)
  {
    for (int j = 0; j < ny - 1; j++)
    {
      // Different signs
      if (!((at(i, j) - T > 0.0) == (at(i, j + 1) - T > 0.0)))
      {
        v.append(Dichotomy(ArrayVertex(i, j), ArrayVertex(i, j + 1), at(i, j) - T, at(i, j + 1) - T, celldiagonal[1], T, epsilon));
        eay[VertexIndex(i, j)] = nv;
        nv++;
      }
    }
  }

  // Array for edge vertices
  QVector<int> edges;

  // There may be no more than nv segments
  edges.reserve(2 * nv);

  // Create edges for interior cells
  for (int i = 0; i < nx - 1; i++)
  {
    for (int j = 0; j < ny - 1; j++)
    {
      int squareindex = 0;
      if (at(i, j) - T > 0.0) squareindex |= 1;
      if (at(i + 1, j) - T > 0.0) squareindex |= 2;
      if (at(i, j + 1) - T > 0.0) squareindex |= 4;
      if (at(i + 1, j + 1) - T > 0.0) squareindex |= 8;

      switch (squareindex)
      {
      case 0:
        break;
      case 1:
        edges.append(eay[VertexIndex(i, j)]);
        edges.append(eax[VertexIndex(i, j)]);
        break;
      case 2:
        edges.append(eay[VertexIndex(i + 1, j)]);
        edges.append(eax[VertexIndex(i, j)]);
        break;
      case 3:
        edges.append(eay[VertexIndex(i + 1, j)]);
        edges.append(eay[VertexIndex(i, j)]);
        break;
      case 4:
        edges.append(eay[VertexIndex(i, j)]);
        edges.append(eax[VertexIndex(i, j + 1)]);
        break;
      case 5:
        edges.append(eax[VertexIndex(i, j)]);
        edges.append(eax[VertexIndex(i, j + 1)]);
        break;
      case 6:
        edges.append(eay[VertexIndex(i, j)]);
        edges.append(eax[VertexIndex(i, j + 1)]);
        edges.append(eay[VertexIndex(i + 1, j)]);
        edges.append(eax[VertexIndex(i, j)]);
        break;
      case 7:
        edges.append(eay[VertexIndex(i + 1, j)]);
        edges.append(eax[VertexIndex(i, j + 1)]);
        break;
      case 8:
        edges.append(eax[VertexIndex(i, j + 1)]);
        edges.append(eay[VertexIndex(i + 1, j)]);
        break;
      case 9:
        edges.append(eax[VertexIndex(i, j)]);
        edges.append(eay[VertexIndex(i, j)]);
        edges.append(eax[VertexIndex(i, j + 1)]);
        edges.append(eay[VertexIndex(i + 1, j)]);
        break;
      case 10:
        edges.append(eax[VertexIndex(i, j + 1)]);
        edges.append(eax[VertexIndex(i, j)]);
        break;
      case 11:
        edges.append(eax[VertexIndex(i, j + 1)]);
        edges.append(eay[VertexIndex(i, j)]);
        break;
      case 12:
        edges.append(eay[VertexIndex(i, j)]);
        edges.append(eay[VertexIndex(i + 1, j)]);
        break;
      case 13:
        edges.append(eax[VertexIndex(i, j)]);
        edges.append(eay[VertexIndex(i + 1, j)]);
        break;
      case 14:
        edges.append(eay[VertexIndex(i, j)]);
        edges.append(eax[VertexIndex(i, j)]);
        break;
      case 15:
        break;
      };
    }
  }

  // Escape if no boundary
  if (closed == false)
    return SegmentSet2(v, edges);

  // Border vertexes
  const int nb = VertexBorderSize();

  QVector<int> vxy(nb);

  for (int k = 0; k < nb; k++)
  {
    if (at(VertexBorder(k)) - T > 0.0)
    {
      v.append(ArrayVertex(VertexBorder(k).x(), VertexBorder(k).y()));
      vxy[k] = nv;
      nv++;
    }
  }

  // Create edges for boundary

  // Horizontal 
  for (int i = 0; i < nx - 1; i++)
  {
    // Straddling edge
    if (eax[VertexIndex(i, 0)] >= 0)
    {
      if (at(i, 0) - T > 0.0)
      {
        edges.append(vxy[VertexBorderIndex(i, 0)]);
        edges.append(eax[VertexIndex(i, 0)]);
      }
      else
      {
        edges.append(eax[VertexIndex(i, 0)]);
        edges.append(vxy[VertexBorderIndex(i + 1, 0)]);
      }
    }
    else
    {
      if ((at(i, 0) - T > 0.0) && (at(i + 1, 0) - T > 0.0))
      {
        edges.append(vxy[VertexBorderIndex(i, 0)]);
        edges.append(vxy[VertexBorderIndex(i + 1, 0)]);
      }
    }
  }

  for (int i = 0; i < nx - 1; i++)
  {
    // Straddling edge
    if (eax[VertexIndex(i, ny - 1)] >= 0)
    {
      if (at(i, ny - 1) - T > 0.0)
      {
        edges.append(vxy[VertexBorderIndex(i, ny - 1)]);
        edges.append(eax[VertexIndex(i, ny - 1)]);
      }
      else
      {
        edges.append(eax[VertexIndex(i, ny - 1)]);
        edges.append(vxy[VertexBorderIndex(i + 1, ny - 1)]);
      }
    }
    else
    {
      if ((at(i, ny - 1) - T > 0.0) && (at(i + 1, ny - 1) - T > 0.0))
      {
        edges.append(vxy[VertexBorderIndex(i, ny - 1)]);
        edges.append(vxy[VertexBorderIndex(i + 1, ny - 1)]);
      }
    }
  }

  // Vertical 
  for (int j = 0; j < ny - 1; j++)
  {
    // Straddling edge
    if (eay[VertexIndex(0, j)] >= 0)
    {
      if (at(0, j) - T > 0.0)
      {
        edges.append(vxy[VertexBorderIndex(0, j)]);
        edges.append(eay[VertexIndex(0, j)]);
      }
      else
      {
        edges.append(eay[VertexIndex(0, j)]);
        edges.append(vxy[VertexBorderIndex(0, j + 1)]);
      }
    }
    else
    {
      if ((at(0, j) - T > 0.0) && (at(0, j + 1) - T > 0.0))
      {
        edges.append(vxy[VertexBorderIndex(0, j)]);
        edges.append(vxy[VertexBorderIndex(0, j + 1)]);
      }
    }
  }

  for (int j = 0; j < ny - 1; j++)
  {
    // Straddling
    if (eay[VertexIndex(nx - 1, j)] >= 0)
    {
      if (at(nx - 1, j) - T > 0.0)
      {
        edges.append(vxy[VertexBorderIndex(nx - 1, j)]);
        edges.append(eay[VertexIndex(nx - 1, j)]);
      }
      else
      {
        edges.append(eay[VertexIndex(nx - 1, j)]);
        edges.append(vxy[VertexBorderIndex(nx - 1, j + 1)]);
      }
    }
    else
    {
      if ((at(nx - 1, j) - T > 0.0) && (at(nx - 1, j + 1) - T > 0.0))
      {
        edges.append(vxy[VertexBorderIndex(nx - 1, j)]);
        edges.append(vxy[VertexBorderIndex(nx - 1, j + 1)]);
      }
    }
  }
  return SegmentSet2(v, edges);

}

/*!
\brief Compute the intersection between a segment and the scalar field.

\param a,b End vertices of the segment straddling the surface.
\param va,vb Field function value at those end vertices.
\param length Distance between vertices.
\param epsilon Precision.
\param T Threshold value.
\return Point.
*/
Vector2 ScalarField2::Dichotomy(Vector2 a, Vector2 b, double va, double vb, double length, const double& T, const double& epsilon) const
{
  int ia = va > 0 ? 1 : -1;

  // Get an accurate first guess
  Vector2 c = Vector2::Solve(a, b, va, vb);

  while (length > epsilon)
  {
    double vc = Value(c) - T;
    int ic = vc > 0.0 ? 1 : -1;
    if (ia + ic == 0)
    {
      b = c;
    }
    else
    {
      ia = ic;
      a = c;
    }
    length *= 0.5;
    c = 0.5 * (a + b);
  }
  return c;
}
