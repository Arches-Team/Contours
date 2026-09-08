
#include "sampling.h"
#include "cubic.h"

/*!
\brief Initialize a poisson disc tiling.
*/
DiscTile::DiscTile()
{
}

/*!
\brief Create a poisson sampling in a square domain.
\param s Size of the side of the square.
\param r Radius of the samples.
\param n Maximum number of samples in the dart throwing process. Note that the number of samples will be different.
\param ra %Random number generator.
*/
DiscTile::DiscTile(const double& s, const double& r, int n, Random& ra) :s(s), r(r), e(r* r * 4.02)
{
  DiscTile::Generate(n, ra);
}

/*!
\brief Scale a disc tile.
\param s Scaling factor.
*/
void DiscTile::Scale(const double& s)
{
  r *= s;
  DiscTile::s *= s;
  e = (r * r * 4.02);
  for (Vector2& pi : p)
  {
    pi *= s;
  }
}

/*!
\brief Perform a relaxation step over the Poisson-Disc distribution.
\param c Relaxation coefficient, it the fraction of the radius of the Poisson distribution.
\param n Number of steps.
*/
void DiscTile::Relaxation(int n, const double& c)
{
  for (int i = 0; i < n; i++)
  {
    Relaxation(c);
  }
}

/*!
\brief Perform a relaxation step over the Poisson-Disc distribution.
\param c Relaxation coefficient, it is the fraction of the Poisson radius.
*/
void DiscTile::Relaxation(const double& c)
{
  // Size of the square
  const double t = 2.0 * r / sqrt(2.0);
  int mod = int(s / t) + 1;

  // Grid for speeding up queries
  QVector<int> grid(mod * mod);
  grid.fill(-1, mod * mod);

  for (int k = 0; k < p.size(); k++)
  {
    int xk = (p.at(k)[0] * mod / s);
    int yk = (p.at(k)[1] * mod / s);
    grid[xk * mod + yk] = k;
  }

  // Displacements
  QVector<Vector2> d(p.size());

  // Compute displacements
  for (int k = 0; k < p.size(); k++)
  {
    Vector2 eps = Vector2::Null;
    int xk = (p.at(k)[0] * mod / s);
    int yk = (p.at(k)[1] * mod / s);
    for (int i = -3; i < 4; i++)
    {
      for (int j = -3; j < 4; j++)
      {
        int ox = 0;
        int oy = 0;

        if (i + xk < 0) { ox = -1; }
        if (i + xk >= mod) { ox = 1; }
        if (j + yk < 0) { oy = -1; }
        if (j + yk >= mod) { oy = 1; }

        int gi = (i + xk + mod) % mod;
        int gj = (j + yk + mod) % mod;
        int ng = grid.at(gi * mod + gj);
        if ((ng != -1) && (ng != k))
        {
          Vector2 dis = p.at(ng) + Vector2(ox * s, oy * s) - p.at(k);
          double dd = dis * dis;

          // Move by a small amount of radius, smoothed according to distance
          eps += -r * c * Normalized(dis) * Cubic::SmoothCompact(dd, 16.0 * r * r);
        }
      }
    }
    // Move
    d[k] = eps;
  }

  // Apply displacements
  for (int k = 0; k < p.size(); k++)
  {
    p[k] += d[k];

    // Keep point within tile
    p[k] = Vector2::Mod(p[k], Vector2(s, s));
  }
}

//   // Toric space, moreover the point is close to the boundary

/*!
\brief Generate the poisson sampling of the domain.
\param n Number of darts thrown on the domain. Note that the number of generated samples will be less than n.
\param random %Random number generator.
*/
void DiscTile::Generate(int n, Random& random)
{
  // Size of the square
  double t = 2.0 * r / sqrt(2.0);
  int mod = int(s / t) + 1;

  QVector<int> grid(mod * mod, -1);

  // Locate existing samples into the grid
  for (int k = 0; k < p.size(); k++)
  {
    double x = p.at(k)[0];
    double y = p.at(k)[1];

    int xk = (x * mod / s);
    int yk = (y * mod / s);

    grid[xk * mod + yk] = k;
  }
  static const QPoint set[25] = {
    QPoint(-2,-2),QPoint(-1,-2), QPoint(0,-2), QPoint(1,-2), QPoint(2,-2),
    QPoint(-2,-1),QPoint(-1,-1), QPoint(0,-1), QPoint(1,-1), QPoint(2,-1),
    QPoint(-2, 0),QPoint(-1, 0), QPoint(0, 0), QPoint(1, 0), QPoint(2, 0),
    QPoint(-2,+1),QPoint(-1,+1), QPoint(0,+1), QPoint(1,+1), QPoint(2,+1),
    QPoint(-2,+2),QPoint(-1,+2), QPoint(0,+2), QPoint(1,+2), QPoint(2,+2)
  };
  // Try to sample with n discs
  for (int k = 0; k < n; k++)
  {
    double x = random.Uniform(s);
    double y = random.Uniform(s);

    int xk = (x * mod / s);
    int yk = (y * mod / s);

    // Detect collisions with existing discs
    bool c = false;

    for (int l = 0; l < 25; l++)
    {
      int i = set[l].x();
      int j = set[l].y();
      int ox = 0;
      int oy = 0;

      if (i + xk < 0) { ox = -1; }
      if (i + xk >= mod) { ox = 1; }
      if (j + yk < 0) { oy = -1; }
      if (j + yk >= mod) { oy = 1; }

      int gi = (i + xk + 2 * mod) % mod;
      int gj = (j + yk + 2 * mod) % mod;
      int ng = grid.at(gi * mod + gj);
      if (ng != -1)
      {
        if (SquaredNorm(p.at(ng) + Vector2(ox * s, oy * s) - Vector2(x, y)) < e)
        {
          c = true;
          break;
        }
      }
      if (c == true) break;

    }

    if (c == false)
    {
      grid[xk * mod + yk] = p.size();
      p.append(Vector2(x, y));
    }
  }
}

