// Heightfield

#include "heightfield.h"
#include "turbulence.h"
#include "ms_spe_gpu.h"
#include "gputerrainsimu.h"
#include "gpusediment.h"

const double HeightField::flat = 1.0e-8;

/*!
\brief Create a flat heightfield.
\param box Rectangle domain of the terrain.
\param nx, ny Samples.
\param v Constant elevation.
*/
HeightField::HeightField(const Box2& box, int nx, int ny, const double& v) : ScalarField2(box, nx, ny, v)
{
}

/*!
\brief Create a heightfield from a scalar field.

This constructor provides implicit conversion.

\param s Scalar field.
*/
HeightField::HeightField(const ScalarField2& s) : ScalarField2(s)
{
}

/*!
\brief Create a heightfield from an image.
\param box Rectangle domain of the terrain.
\param image Elevation image.
\param a, b Minimum and maximum elevation range.
\param grayscale Boolean set to false if the image is provided in color.
*/
HeightField::HeightField(const Box2& box, const QImage& image, const double& a, const double& b, bool grayscale) : ScalarField2(box, image, a, b, grayscale)
{
}

/*!
\brief Compute the vertex corresponding to a given sample.
\param i, j Integer coordinates of the sample.
*/
Vector HeightField::Vertex(int i, int j) const
{
  return Vector(a[0] + i * celldiagonal[0], a[1] + j * celldiagonal[1], at(i, j));
}

/*!
\brief Compute the stream area field of the terrain.

The stream area is computed as a fraction of cells, independtly of the surface area of a cell.
\param power Power.
*/
ScalarField2 HeightField::StreamArea(const double& power) const
{
  ScalarField2 stream(Box2(a, b), nx, ny, 1.0);

  QVector<ScalarPoint2> queue = GetScalarPoints();
  std::sort(queue.begin(), queue.end());

  for (int i = queue.size() - 1; i >= 0; i--)
  {
    QPoint p = queue.at(i).Point();

    FlowStruct flow;
    int n = CheckFlowSlope(p, flow, power);
    if (n > 0)
    {
      const double sp = stream(p);
      for (int j = 0; j < n; j++)
      {
        stream(flow.q[j]) += sp * flow.sn[j];
      }
    }
  }

  return stream;
}

/*!
\brief Compute the maximum slope field of the terrain.

Simply compute the norm of the gradient.

This function may be used to perform hill-slope erosion simulation.

\param a Boolean, use average slope if set to true, and slope otherwize.
\sa (), Slope(), AverageSlope(), GradientNorm()
*/
ScalarField2 HeightField::Slope(bool a) const
{
  if (a == true)
  {
    return AverageSlope();
  }
  else
  {
    return GradientNorm();
  }
}

/*!
\brief Compute the average slope field of the terrain.
*/
ScalarField2 HeightField::AverageSlope() const
{
  ScalarField2 slope(Box2(a, b), nx, ny);
  for (int i = 0; i < nx; i++)
  {
    for (int j = 0; j < ny; j++)
    {
      slope(i, j) = AverageSlope(i, j);
    }
  }
  return slope;
}

/*!
\brief Compute the average slope at a given integer point on the terrain.

Simply average the slope in 8 directions.
\param i, j Integer coordinates
*/
double HeightField::AverageSlope(int i, int j) const
{
  double s;

  double e = Norm(celldiagonal);

  if (i == 0)
  {
    if (j == 0)
    {
      // Corner
      s = fabs(at(i, j) - at(i + 1, j)) * inversecelldiagonal[0] + fabs(at(i, j) - at(i + 1, j + 1)) / e + fabs(at(i, j) - at(i, j + 1)) * inversecelldiagonal[1];
      s /= 3.0;
    }
    else if (j == ny - 1)
    {
      // Corner
      s = fabs(at(i, j) - at(i + 1, j)) * inversecelldiagonal[0] + fabs(at(i, j) - at(i + 1, j - 1)) / e + fabs(at(i, j) - at(i, j - 1)) * inversecelldiagonal[1];
      s /= 3.0;
    }
    else
    {
      // Edge
      s = fabs(at(i, j) - at(i, j - 1)) * inversecelldiagonal[1] + fabs(at(i, j) - at(i + 1, j - 1)) / e + fabs(at(i, j) - at(i + 1, j)) * inversecelldiagonal[0] + fabs(at(i, j) - at(i + 1, j + 1)) / e + fabs(at(i, j) - at(i, j + 1)) * inversecelldiagonal[1];
      s /= 5.0;
    }
  }
  else if (i == nx - 1)
  {
    if (j == 0)
    {
      // Corner
      s = fabs(at(i, j) - at(i - 1, j)) * inversecelldiagonal[0] + fabs(at(i, j) - at(i - 1, j + 1)) / e + fabs(at(i, j) - at(i, j + 1)) * inversecelldiagonal[1];
      s /= 3.0;
    }
    else if (j == ny - 1)
    {
      // Corner
      s = fabs(at(i, j) - at(i - 1, j)) * inversecelldiagonal[0] + fabs(at(i, j) - at(i - 1, j - 1)) / e + fabs(at(i, j) - at(i, j - 1)) * inversecelldiagonal[1];
      s /= 3.0;
    }
    else
    {
      // Edge
      s = fabs(at(i, j) - at(i, j - 1)) * inversecelldiagonal[1] + fabs(at(i, j) - at(i - 1, j - 1)) / e + fabs(at(i, j) - at(i - 1, j)) * inversecelldiagonal[0] + fabs(at(i, j) - at(i - 1, j + 1)) / e + fabs(at(i, j) - at(i, j + 1)) * inversecelldiagonal[1];
      s /= 5.0;
    }
  }
  else
  {
    if (j == 0)
    {
      // Edge
      s = fabs(at(i, j) - at(i - 1, j)) * inversecelldiagonal[0] + fabs(at(i, j) - at(i - 1, j + 1)) / e + fabs(at(i, j) - at(i, j + 1)) * inversecelldiagonal[1] + fabs(at(i, j) - at(i + 1, j + 1)) / e + fabs(at(i, j) - at(i + 1, j)) * inversecelldiagonal[0];
      s /= 5.0;
    }
    else if (j == ny - 1)
    {
      // Edge
      s = fabs(at(i, j) - at(i - 1, j)) * inversecelldiagonal[0] + fabs(at(i, j) - at(i - 1, j - 1)) / e + fabs(at(i, j) - at(i, j - 1)) * inversecelldiagonal[1] + fabs(at(i, j) - at(i + 1, j - 1)) / e + fabs(at(i, j) - at(i + 1, j)) * inversecelldiagonal[0];
      s /= 5.0;
    }
    else
    {
      // Vertex
      s = fabs(at(i, j) - at(i + 1, j)) * inversecelldiagonal[0] + fabs(at(i, j) - at(i + 1, j + 1)) / e + fabs(at(i, j) - at(i, j + 1)) * inversecelldiagonal[1] + fabs(at(i, j) - at(i - 1, j + 1)) / e + fabs(at(i, j) - at(i - 1, j)) * inversecelldiagonal[0] + fabs(at(i, j) - at(i - 1, j - 1)) / e + fabs(at(i, j) - at(i, j - 1)) * inversecelldiagonal[1] + fabs(at(i, j) - at(i + 1, j - 1)) / e;
      s /= 8.0;
    }
  }
  return s;
}

/*!
\brief Get the bounding box of the heightfield.

Although this function has the same name as Array2::GetBox(),
it computes the minimum and maximum elevation of the terrain (computationally intensive).

The two-dimensional box can be obtained by using:
\code
HeightField heightfield;
Box2 box = heightfield.Array2::GetBox(); // Get the domain in the plane.
\endcode

\sa Array2::GetBox()
*/
Box HeightField::GetBox() const
{
  double za, zb;
  GetRange(za, zb);
  return Array2::GetBox().ToBox(za, zb);
}

/*!
\brief Scale the height field.

The domain is scaled using the x and y components of the scaling vector,
whereas the heights of the heightfield are scaled using the z component.
\param s Scaling factor.
*/
void HeightField::Scale(const Vector& s)
{
  // Box
  ScalarField2::Scale(Vector2(s));

  // Heights
  for (int i = 0; i < field.size(); i++)
  {
    field[i] *= s[2];
  }
}

/*!
\brief Applies three erosion steps on gpu : stream power, slope stabilization, deposition
\param e Standard is 1, can be adjusted between 0 and +inf.
\param d Deposition strength
*/
void HeightField::ErosionDeposition(double e, double d)
{
  ScalarField2 hardness_map = NoiseTurbulence2(0., 1., 5000., 0.7, 0.4, 5, Vector(rand(), -rand(), rand())).Sample(*this);
  hardness_map.Normalize();
  MS_SPE_GPU gpu_erosion;
  gpu_erosion.Init(*this);
  gpu_erosion.SetK(0.01f);
  gpu_erosion.SetSmoothSteps(0);
  gpu_erosion.SetHardness(hardness_map);
  gpu_erosion.Step(int(e * 250));
  gpu_erosion.GetResult(*this);

  GPUThermalErosionSimple gpu_thermal;
  gpu_thermal.Init(*this);
  gpu_thermal.SetUniforms(0.0005f, 3.0f, true, 0.0023f);
  gpu_thermal.Step(3000);
  gpu_thermal.GetHeight(*this);

  GPU_Deposition gpu_depo;
  gpu_depo.Init(*this);
  gpu_depo.SetUniforms(1.);
  gpu_depo.Step(int(d * 1000));
  gpu_depo.GetHeight(*this);
}

/*!
\brief Amplification with multiscale erosion.
\param n Number of steps.
\param e, d Erosion and deposition coefficients.
*/
void HeightField::ErosionAmpli(int n, double e, double d)
{
  const int factor = 2;

  for (int i = 0; i < n; i++)
  {
    ErosionDeposition(e * (1.0 / (1.0 + i)), d * (1.0 / (1.0 + 1.5 * i)));
    if (i == n - 1) break;
    // x2 upsampling
    int current_size = GetSizeX();
    *this = Resized(current_size * factor, current_size * factor, false);
  }
}

