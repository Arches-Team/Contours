#include "misc.h"
#include "sampling.h"
#include "tin.h"

/*
 * Fait un DiskSampling delaunaysé à l'intérieur de box
 */
Mesh2 Misc::DelaunayPointsInBox(const Box2& box, double r)
{
	// TODO use a precomputed one
	DiscTileLinked sample(70, 1, 10000);

	double ratio = r / sample.Radius();
	sample.Scale(ratio);

	// Get all triangles of the sample
	// attention la technique ne marche pas si on peut faire une boucle de 3 sommets qui n'est pas un triangle dans le sample
	// a priori ça m'arrive que si le sample est trop petit
	QVector<QPoint> pointIndices;
	int n = sample.Size();

	for (int ind = 0; ind < n; ++ind)
	{
		QPoint pi1(ind, 8);
		for (int neigh1 = 0; neigh1 < sample.Valence(ind); ++neigh1)
		{
			QPoint pi2 = sample.Next(ind, neigh1);
			if (pi2.x() > pi1.x())
			{
				for (int neigh2 = neigh1 + 1; neigh2 < sample.Valence(ind); ++neigh2)
				{
					QPoint pi3 = sample.Next(ind, neigh2);
					if (pi3.x() > pi1.x())
					{
						// Currently we know that pi1 is attached to pi2 and pi3, but we dont know if pi2 and pi3 are neighbours, so we check it out
						bool isTriangle = false;
						for (int neigh3 = 0; neigh3 < sample.Valence(pi2.x()); ++neigh3)
						{
							QPoint pi3bis = sample.Next(pi2.x(), neigh3);
							if (pi3bis.x() == pi3.x())
							{
								isTriangle = true;
								break;
							}
						}

						if (isTriangle)
						{
							// We only add a triangle if pi1 is the smallest indices, so the triangle is added only once
							pointIndices.append(pi1);
							pointIndices.append(pi2);
							pointIndices.append(pi3);
						}
					}
				}
			}
		}
	}

	// Copied from DiscTileLinked::next
	const QPoint next[9] = { QPoint(1,0),QPoint(1,1),QPoint(0,1),QPoint(-1,1),QPoint(-1,0),QPoint(-1,-1),QPoint(0,-1),QPoint(1,-1),QPoint(0,0) };

	// Now we can create the Mesh

	double w = sample.GetBox().Width();

	double W = box[1][0] - box[0][0];
	double H = box[1][1] - box[0][1];
	int nW = Math::Ceil(W / w);
	int nH = Math::Ceil(H / w);

	QVector<Vector2> points(n * nW * nH);
	QVector<int> indices;

	for (int i = 0; i < nW; ++i)
	{
		double x = box[0][0] + i * w;
		for (int j = 0; j < nH; ++j)
		{
			double y = box[0][1] + j * w;
			Vector2 t(x, y);
			int boxInd = i * nH + j;

			// We add n points
			for (int p = 0; p < n; ++p)
			{
				int ind = boxInd * n + p;
				points[ind] = sample.Vertex(p) + t;
			}

			// We also add triangles
			for (int t = 0; t < pointIndices.size(); t += 3)
			{
				QPoint pi1 = pointIndices[t];
				QPoint pi2 = pointIndices[t + 1];
				QPoint pi3 = pointIndices[t + 2];

				QPoint tile1 = QPoint(i, j) + next[pi1.y()];
				QPoint tile2 = QPoint(i, j) + next[pi2.y()];
				QPoint tile3 = QPoint(i, j) + next[pi3.y()];

				if (tile1.x() < 0 || tile1.x() >= nW || tile1.y() < 0 || tile1.y() >= nH)
					continue;
				if (tile2.x() < 0 || tile2.x() >= nW || tile2.y() < 0 || tile2.y() >= nH)
					continue;
				if (tile3.x() < 0 || tile3.x() >= nW || tile3.y() < 0 || tile3.y() >= nH)
					continue;

				int ind1 = (tile1.x() * nH + tile1.y()) * n + pi1.x();
				int ind2 = (tile2.x() * nH + tile2.y()) * n + pi2.x();
				int ind3 = (tile3.x() * nH + tile3.y()) * n + pi3.x();

				indices.append(ind1);
				indices.append(ind2);
				indices.append(ind3);
			}
		}
	}

	return Mesh2(points, indices).SubMesh(box);
}

/*
 * When two mesh are submeshes, indices completly change
 * This function allows one to get the matching of one mesh indices to the other
 * O(n) amorti
 * 
 * \param m1 The mesh we have indices
 * \param m2 The mesh we want to have correlation
 * \param mindist A minimum distance between two vertices in m1 (for position comparison)
 * 
 * \return A vector Match, where for all index i in m1, we have Match[i] = j for j its matching in m2 (-1 if no matching)
 */
QVector<int> Misc::IndicesBetweenMeshes(const Mesh2& m1, const Mesh2& m2, const double& mindist)
{
	QVector<int> match(m1.VertexSize(), -1);
	QMap<QPair<int, int>, QVector<int>> map;

	// On construit des cases de taille mindist/mindist
	for (int vi1 = 0; vi1 < m1.VertexSize(); ++vi1)
	{
		Vector2 v1 = m1.Vertex(vi1);
		int first = (int) v1[0] / mindist;
		int second = (int) v1[1] / mindist;

		QPair<int, int> p(first, second);
		if (!map.contains(p))
			map[p] = {};
		map[p].append(vi1);
	}

	// On cherche le sommet le plus proche dans la case et les cases adjacentes
	for (int vi2 = 0; vi2 < m2.VertexSize(); ++vi2)
	{
		Vector2 v2 = m2.Vertex(vi2);
		int first = (int) v2[0] / mindist;
		int second = (int) v2[1] / mindist;
		bool find = false;

		int checkDist = 1;

		for (int x = first - checkDist; x <= first + checkDist; ++x)
		{
			for (int y = second - checkDist; y <= second + checkDist; ++y)
			{
				QPair<int, int> p(x, y);
				if (map.contains(p))
				{
					for (int vi1 : map[p])
					{
						Vector2 v1 = m1.Vertex(vi1);
						if (Norm(v1 - v2) < mindist)
						{
							match[vi1] = vi2;
							find = true;
							break;
						}
					}
				}
				if (find) break;
			}
			if (find) break;
		}
	}

	// Versione n O(n^2) pas ouf
	//for (int vi1 = 0; vi1 < m1.VertexSize(); ++vi1)
	//{
	//	Vector2 v1 = m1.Vertex(vi1);
	//	for (int vi2 = 0; vi2 < m2.VertexSize(); ++vi2)
	//	{
	//		Vector2 v2 = m2.Vertex(vi2);
	//		if (Norm(v1 - v2) < mindist)
	//		{
	//			match[vi1] = vi2;
	//			break;
	//		}
	//	}
	//}

	return match;
}