// Heightfield
#include "heightfield.h"

/*!
\brief Compute the flow directions at a given point, using an L<SUP>2</SUP> metric.

\param p Point.
\param flow Flow information.
\param power Power, set to 1.0 for diffusive flow routing, converges to steepest slope as power increases to infinity.
*/
int HeightField::CheckFlowSlope(const QPoint& p, FlowStruct& flow, const double& power) const
{
  int n = 0;

  double zp = at(p);

  flow.mask = 0;

  for (int i = 0; i < 8; i++)
  {
    QPoint b = p + next[i];
    // Skip if point is not inside the domain
    if (!InsideVertexIndex(b))
    {
      continue;
    }

    double step = at(b) - zp;
    if (step < -HeightField::flat) // Should be 0.0, but very small negative values might crash
    {
      flow.mask |= 1 << i;
      flow.i[n] = i;
      flow.q[n] = b;
      flow.h[n] = -step;
      flow.s[n] = -step * inverselength[i];

      // Steepest slope
      if (n == 0)
      {
        flow.steepest = 0;
      }
      else if (flow.s[n] > flow.s[flow.steepest])
      {
        flow.steepest = n;
      }
     
      n++;
    }
  }

  // Weighted slope
  double slopesum = 0.0;
  if (power==1.0)
  { 
    for (int k = 0; k < n; k++)
    {
      slopesum += flow.s[k], power;
    }
    // Relative weighted squared slopes
    for (int k = 0; k < n; k++)
    {
      flow.sn[k] = flow.s[k] / slopesum;
    }
  }
  else
  {
    for (int k = 0; k < n; k++)
    {
      slopesum+= Math::Pow(flow.s[k], power);
    }

    // Relative weighted squared slopes
    for (int k = 0; k < n; k++)
    {
      flow.sn[k] = Math::Pow(flow.s[k], power) / slopesum;
    }
  }

  return n;
}
