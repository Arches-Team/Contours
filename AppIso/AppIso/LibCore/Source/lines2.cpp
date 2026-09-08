// Set of line segments

#include "segment.h"
#include "curvepoint.h"
#include "polygon.h"

/*!
\brief Empty.
*/
SegmentSet2::SegmentSet2()
{
}

/*!
\brief Create a set of line segments from a list of vertices and a indexes.

Indices should be a multiple of two; the number of segments is derived
from the size of the array.

\sa Mesh2

\param v Set of vertices.
\param i Indexes that represent the segments.
*/
SegmentSet2::SegmentSet2(const QVector<Vector2>& v, const QVector<int>& i) : vertices(v), indices(i)
{
}

/*!
\brief Get the subset of closed polygons.

Detects all loops in the structure and create closed polygons, open polylines are not returned.

\param linear Select either linear (true) or quadratic (false, default) complexity.
*/
Polygons2 SegmentSet2::GetPolygons(bool linear) const
{
  // Set of closed polygons
  Polygons2 ps;

  // Set of open curves
  QVector <PointCurve2> pc;

  if (linear == false)
  {
    QVector<int> queue = indices;

    while (!queue.empty())
    {
      // Take two elements that form the start of a polygon or a curve
      int iib = queue.last();
      queue.pop_back();
      int iia = queue.last();
      queue.pop_back();

      // Create a tentative loop of indexes
      QVector<int> loop;
      loop.append(iia);
      loop.append(iib);

      bool finished = false;
      while (!finished)
      {
        bool attached = false;
        // Find if another segment can be attached
        for (int j = 0; j < queue.size(); j += 2)
        {
          int ja = queue.at(j);
          int jb = queue.at(j + 1);

          // Attach from this side
          if (ja == loop.at(0))
          {
            attached = true;
            if (jb == loop.at(loop.size() - 1))
            {
              // Both ends match: close polygon
              ps.Add(Polygon2(vertices, loop));
              finished = true;
            }
            else
            {
              // Only attach
              loop.prepend(jb);
            }
            // Remove candidates : they have been attached
            queue.remove(j, 2);
            break;
          }
          // Or at the end
          else if (jb == loop.at(0))
          {
            attached = true;
            if (ja == loop.at(loop.size() - 1))
            {
              // We have to check if the polygon is given in the trigo order as we need
              double area = 0.0;

              for (int i = 0; i < loop.size(); i++)
              {
                const Vector2& pa = vertices.at(loop[i]);
                const Vector2& pb = vertices.at(loop[(i + 1) % loop.size()]);

                area += pa[0] * pb[1] - pb[0] * pa[1];
              }

              if (area < 0.0)
              {
                std::reverse(loop.begin(), loop.end());
              }

              // Both ends match: close polygon
              ps.Add(Polygon2(vertices, loop));
              finished = true;
            }
            else
            {
              // Only attach
              loop.prepend(ja);
            }
            // Remove candidates : they have been attached
            queue.remove(j, 2);
            break;
          }
        }
        // We traversed all the set and could not attach any segment: this means the loop cannot be closed
        if (attached == false)
        {
          pc.append(PointCurve2(vertices, loop));
          finished = true;
        }
      }
    }
  }
  else
  {
    // Keep first and second reference of each vertices (in edges)
    // This algo does not work if there is more than 2 segments attached to one vertex
    QVector<int> first(vertices.size(), -1);
    QVector<int> second(vertices.size(), -1);

    for (int i = 0; i < indices.size(); ++i)
    {
      if (first[indices[i]] == -1)
        first[indices[i]] = i;
      else if (second[indices[i]] == -1)
        second[indices[i]] = i;
      else
      {
        std::cerr << "[warning]: cannot find polygons if more than 2 segments intersect in one vertex." << std::endl;
        return ps;
      }
    }

    QVector<bool> passed(vertices.size(), false);

    for (int i = 0; i < vertices.size(); ++i)
    {
      if (passed[i]) continue;

      passed[i] = true;

      int e = first[i];
      // No segment ends up in i, thus no polygons
      if (e == -1) continue;

      QVector<int> loop;
      loop.append(i);

      // We take the other vertex of the edge
      if (e % 2 == 0) e++;
      else            e--;

      int j = indices[e];
      bool firstEndPointFound = second[i] == -1; // if i has no other point, we know i is the endpoint of an open curve

      // This loop works like this:
      // - e keeps track of the current edge
      // - j keeps track of the current vertex
      // we move e from one side of the edge to the other, then we switch j according to what vertex we find
      while (j != i)
      {
        passed[j] = true;
        loop.append(j);

        e = first[j] == e ? second[j] : first[j];
        // if second[j] == -1, it means that j is an end point of an open curve
        if (e == -1)
        {
          if (!firstEndPointFound)
          {
            firstEndPointFound = true;
            e = second[i];
            std::reverse(loop.begin(), loop.end()); // We reverse the list since we will add vertices in the other side, and `prepend` is not in O(1)!
          }
          else
          {
            pc.append(PointCurve2(vertices, loop));
            break;
          }
        }

        if (e % 2 == 0) e++;
        else            e--;

        j = indices[e];
      }

      // We have a complete polygons
      if (!firstEndPointFound)
      {
        // We have to check if the polygon is given in the trigo order as we need
        double area = 0.0;

        for (int k = 0; k < loop.size(); k++)
        {
          const Vector2& pa = vertices.at(loop[k]);
          const Vector2& pb = vertices.at(loop[(k + 1) % loop.size()]);

          area += pa[0] * pb[1] - pb[0] * pa[1];
        }

        if (area < 0.0)
        {
          std::reverse(loop.begin(), loop.end());
        }

        ps.Add(Polygon2(vertices, loop));
      }
    }
  }
  return ps;
}
