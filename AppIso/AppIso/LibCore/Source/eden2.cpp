// Eden growth

#include "eden.h"

/*!
\class EdenGrowth2 eden.h

\brief Off-lattice Eden growth.

Inherited classes may implement collision with obstacles, simply implementing default member function EdenGrowth2::Collide.
Open-Eden models may be coded by changing the selection of random particles, simply by re-implementing the default member function EdenGrowth2::Select.
*/

Random EdenGrowth2::random;

/*!
\brief Create an Eden growth structure.
\param c Initial particle position.
\param r Radius of particles.
\param n Lattice or pseudo off lattice parameter.
*/
EdenGrowth2::EdenGrowth2(const Vector2& c, const double& r, int n) : ParticleSet2(c, r), circle(c, r), SamplingN(n)
{
  // First particle
  v.append(-1);
  s.append(0);
}

/*!
\brief Test if a candidate particle collides with an obstacle.

This is a default member function that always returns false.

\param p Point, center of particle.
\return false
*/
#pragma warning(push)
#pragma warning(disable: 4100)  
bool EdenGrowth2::Collide(const Vector2& p) const
{
  return false;
}
#pragma warning(pop)

/*!
\brief Return the indices.
*/
QVector<int> EdenGrowth2::GetIndices() const
{
  return v;
}

/*!
\brief Find candidate growth directions from a particle.

This function has an O(n) complexity.

\param q Position of the candidate particle.
\param sites Array of returned growth positions.
\return The number of candidate growth directions.
*/
int EdenGrowth2::GrowthSites(const Vector2& q, Vector2* sites) const
{
  int n = 0;

  // Find a subset of intersecting particles
  const double subhit = Math::Sqr(4.0 * r);
  int subset[16];
  int num = 0;

  for (int j = 0; j < points.size(); j++)
  {
    double rj = SquaredNorm(points.at(j) - q);
    if (rj < subhit)
    {
      subset[num] = j;
      num++;
    }
  }

  // Now check with the subset
  const double hit = 4.0 * r * r;

  for (int i = 0; i < SamplingN; i++)
  {
    double a = Math::Angle(i, SamplingN);
    const double epsilon = 0.075;
    Vector2 qa = q + 2.0 * r * (1.0 + epsilon) * Vector2::Polar(a);

    bool col = false;

    // Using a subset increases performance, but does not improve overall complexity
    for (int j = 0; j < num; j++)
    {
      double rj = SquaredNorm(points.at(subset[j]) - qa);
      if (rj < hit)
      {
        col = true;
        break;
      }
    }
    // Collision with obstacles
    if (Collide(qa) == true)
    {
      col = true;
    }
    if (col == false)
    {
      sites[n] = qa;
      n++;
    }
  }
  return n;
}

/*!
\brief Perform several growth steps.
\param n Number of growth steps.
*/
void EdenGrowth2::Step(int n)
{
  for (int i = 0; i < n; i++)
  {
    Step();
  }
}

/*!
\brief Select a random particle in the front list.
*/
int EdenGrowth2::Select() const
{
  return random.Integer(s.size());
}

//     // Leaf

/*!
\brief Add a new particle to the cluster.

Select a random particle in the existing cluster, and compute candidate growth directions.
Repeat random selection until a valid particle is found. Then select a randomly generated
growth direction and update the cluster.

This function has an O(n) complexity.
*/
void EdenGrowth2::Step()
{
  Vector2* sites = new Vector2[SamplingN];

  // Repeat untill we managed to grow from a particle 
  while (s.size() > 0)
  {
    // Randomly select a particle
    int c = Select();
    int sc = s.at(c);
    Vector2 q = points.at(sc);

    // Need to find if random growth directions exist
    int n = GrowthSites(q, sites);

    // Found at least one direction
    if (n != 0)
    {
      // Add to the list of boundary particles
      s.append(points.size());

      // Add new particle position 
      int si = random.Integer(n);
      points.append(sites[si]);

      // Update ancestors
      v.append(sc);

      circle.Extend(sites[si]);
      // Optimization: we found exactly one free direction
      if (n == 1)
      {
        // Then the selected particle should die
        s[c] = s[s.size() - 1];
        s.removeLast();
      }
      break;
    }
    // It was impossible to grow from the selected particle
    else
    {
      // Remove particle from set of candidates
      // Using remove() preserves order at the cost of O(n) performance, which is not important here
      // s.remove(c); 
      s[c] = s[s.size() - 1];
      s.removeLast();
    }
  }
  delete[]sites;
}

//   // dd = Math::Sqr(nd * Vector2(1.0, 0.0));
//   // proba = 0.05 + 0.95 * dd;

