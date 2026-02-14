#include "graph.h"
#include "draw.h"
#include "misc.h"
#include "histogram.h"
#include <QtWidgets/QAbstractGraphicsShapeItem>

/*
 * Construit un graphe à partir d'un sample de poisson à l'intérieur d'un polygone
 * 
 * \param mask Le masque définissant quelle zone est dans ou hors du graphe (doit être un masque binaire 0 = hors de la zone)
 * \param r	   Le rayon du Poisson Disk Sampling
 * \param v	   La valeur de base sur les sommets du graphe
 */
GraphPoisson::GraphPoisson(const ScalarField2& mask, double r, const double& v) : radius(r), mask(mask)
{
	// On scale un peu pour être plus gros que le masque et éviter les voisins moches à cause de la Delaunaysation
	Box2 b = mask.GetBox();
	Vector2 border(8*r, 8*r);
	b = Box2(b[0] - border, b[1] + border);

	Mesh2 mesh = Misc::DelaunayPointsInBox(b, r);
	Mesh2 mesh2 = mesh.SubMesh(mask); // TODO: léger bug dans cette fonction, certains points en bordure de mer n'auront pas de voisins

	topologyExt = QSharedPointer<Tin2>::create(mesh);
	topology = QSharedPointer<Tin2>::create(mesh2);

	topoToExt = Misc::IndicesBetweenMeshes(*topology, *topologyExt, r / 2);
	extToTopo = Misc::IndicesBetweenMeshes(*topologyExt, *topology, r / 2);

	values.fill(v, topology->VertexSize());
}

/*
 * Initialise les valeurs du graphe avec le bruit donné 
 */
//GraphPoisson::GraphPoisson(const ScalarField2& mask, double r, const ScalarField2& noise) : GraphPoisson(mask, r)
//{
//	SetValueFromScalarField(noise);
//}

//GraphPoisson::GraphPoisson(const GraphPoisson& oth) : radius(oth.radius), mask(oth.mask), topology(oth.topology), topologyExt(oth.topologyExt), topoToExt(oth.topoToExt), extToTopo(oth.extToTopo), values(oth.values)
//{
//
//}

GraphPoisson::GraphPoisson(const GraphPoisson& oth, const double& v) : GraphPoisson(oth)
{
	values.fill(v);
}

void GraphPoisson::SetValueFromScalarField(const ScalarField2& sf)
{
	for (int i = 0; i < Size(); ++i)
	{
		double v = sf.Value(Position(i));
		values[i] = v;
	}
}

/*
 * This method is when we want the graph to have exactly the value given in the cells of sf
 * Otherwise, the `ScalarField2::Value` method can interpolate values
 * 
 * TODO: Pour le moment cette méthode supprime la valeur 0 car on l'utilise de cette manière
 */
void GraphPoisson::SetStrictValueFromScalarField(const ScalarField2& sf)
{
	QSet<int> zeroes;
	double eps = 0.000001;

	for (int i = 0; i < Size(); ++i)
	{
		double v = sf.Closest(Position(i));
		values[i] = v;
		if (v < eps)
			zeroes.insert(i);
	}

	// On ne veut pas de 0, donc on récupère les valeurs des voisins
	while (!zeroes.empty())
	{
		QSet<int> newZeroes;
		for (int id : zeroes)
		{
			// On récupère la plus petite valeur voisine != 0
			double v = 0.0;
			for (int nid : Neighbours(id))
			{
				if (zeroes.contains(nid))
					continue;
				if (v < eps || v > values[nid])
					v = values[nid];
			}
			values[id] = v;
			if (v < eps)
				newZeroes.insert(id);
		}

		// On abandonne si la zone est une petite zone seule pour éviter la boucle infinie
		if (newZeroes.size() == zeroes.size())
			break;
		zeroes = newZeroes;
	}
}

/*
 * Assign value according to the histogram.
 * It means that it will sort the value of the current graph and assign them according to histogram bins.
 */
void GraphPoisson::SetValueFromHistogram(const HistogramD& histo)
{
	QVector<QPair<double, int>> sorted; // keep the id of the sorted values
	for (int i = 0; i < Size(); ++i)
	{
		sorted.append({ At(i), i });
	}
	std::sort(sorted.begin(), sorted.end(), [](const auto& a, const auto& b) { return a.first < b.first; });
	
	// TODO: ne pas passer par là, où alors faire en sorte que le ToHistogram soit plus cohérent, voir ce qui était fait avant dans GetNextHeight (ancien commit)
	Histogram h = histo.ToHistogram(Size());
	int i = 0;
	for (int hi = 0; hi < h.GetSize(); ++hi)
	{
		for (int bi = 0; bi < h.Value(hi); ++bi)
		{
			if (i == Size())
			{
				qDebug() << "The histogram should have exactly" << Size() << "values, but have more.";
				exit(1);
			}
			values[sorted[i].second] = histo.GetKey(hi);
			++i;
		}
	}

	if (i != Size())
	{
		qDebug() << "The histogram should have exactly" << Size() << "values, but have less (" << i << ").";
		exit(1);
	}
}

void GraphPoisson::GetRange(double& min, double& max) const
{
	min = At(0);
	max = At(0);

	for (int node = 1; node < Size(); ++node)
	{
		Math::SetMinMax(At(node), min, max);
	}
}

QSet<double> GraphPoisson::Values() const
{
	QSet<double> values;
	for (int i = 0; i < Size(); ++i)
	{
		values.insert(At(i));
	}
	return values;
}

QVector<double> GraphPoisson::SortedValues() const
{
	QSet<double> values = Values();
	QVector<double> sorted(values.begin(), values.end());
	std::sort(sorted.begin(), sorted.end());
	return sorted;
}

Box2 GraphPoisson::GetBox() const
{
	Vector2 a = Position(0);
	Vector2 b = Position(0);

	for (int i = 1; i < Size(); ++i)
	{
		a[0] = Math::Min(a[0], Position(i)[0]);
		a[1] = Math::Min(a[1], Position(i)[1]);
		b[0] = Math::Max(b[0], Position(i)[0]);
		b[1] = Math::Max(b[1], Position(i)[1]);
	}

	return Box2(a, b);
}

/*
 * Renvoie la valeur d'un point comme une interpolation barycentrique des 3 sommets voisins
 */
double GraphPoisson::Value(const Vector2& p) const
{
	for (int ti = 0; ti < topology->TriangleSize(); ++ti)
	{
		Triangle2 t = topology->GetTriangle(ti);
		if (t.Inside(p))
		{
			Vector b = t.BarycentricCoordinates(p);
			double va = At(topology->index(ti, 0));
			double vb = At(topology->index(ti, 1));
			double vc = At(topology->index(ti, 2));

			return va * b[0] + vb * b[1] + vc * b[2];
		}
	}

	// Outside the domain
	return 0;
}

/*
 * Renvoi un champs scalaire dans la range donnée correspondant aux valeurs interpolées du maillage
 */
ScalarField2 GraphPoisson::Rasterize(const Box2& b, int w, int h) const
{
	ScalarField2 sf(b, w, h);
	for (int i = 0; i < w; ++i)
	{
		for (int j = 0; j < h; ++j)
		{
			double x = Math::Lerp(b[0][0], b[1][0], i / (w - 1.));
			double y = Math::Lerp(b[0][1], b[1][1], j / (h - 1.));
			sf(i, j) = Value(Vector2(x, y));
		}
	}
	return sf;
}

ScalarField2 GraphPoisson::Rasterize(int w, int h) const
{
	return Rasterize(GetBox(), w, h);
}

double GraphPoisson::SeaLevel() const
{
	double min, max;
	GetRange(min, max);
	if (min == max)
		return min - 1;

	double secondMin = max;
	for (int i = 0; i < Size(); ++i)
		if (At(i) > min && At(i) < secondMin)
			secondMin = At(i);

	double seaValue = min - (secondMin - min);
	return seaValue;
}

Polygons2 GraphPoisson::ContourLines(double h) const
{
	double min, max;
	GetRange(min, max);
	double seaValue = SeaLevel();
	double exteriorValue = seaValue - 100 * (max - min + 1); // valeurs très basse

	// On donne les valeurs à topologyExt, on fait le marching triangle sur ce graphe là
	int n = topologyExt->VertexSize();
	QVector<double> valuesExt(n, exteriorValue);
	for (int evi = 0; evi < n; ++evi)
	{
		int vi = extToTopo[evi];
		if (vi != -1)
		{
			valuesExt[evi] = values[vi];
		}
		else if (ExteriorPointInsideMask(evi))
		{
			valuesExt[evi] = seaValue;
		}
		else
		{
			for (int eni : topologyExt->VertexNeighboursVertices(evi))
			{
				int ni = extToTopo[eni];
				if (ni != -1)
				{
					valuesExt[evi] = Math::Max(values[ni], valuesExt[evi]);
				}
			}
		}
	}

	if (h <= seaValue)
	{
		std::cerr << "[GraphPoisson] You ask to get contours of heights " << h << " while the minimum height is " << min << " and the see level is " << seaValue << " your contours will mean nothing." << std::endl;
	}

	QVector<Vector2> points;
	QVector<int> indices;

	QMap<int, int> edgeToPoint;
	int m = topologyExt->TriangleSize();
	
	for (int ti = 0; ti < m; ++ti)
	{
		int inda = topologyExt->index(ti, 0);
		int indb = topologyExt->index(ti, 1);
		int indc = topologyExt->index(ti, 2);

		double va = valuesExt[inda];
		double vb = valuesExt[indb];
		double vc = valuesExt[indc];

		Vector2 pa = topologyExt->Vertex(inda);
		Vector2 pb = topologyExt->Vertex(indb);
		Vector2 pc = topologyExt->Vertex(indc);

		bool sa = va >= h;
		bool sb = vb >= h;
		bool sc = vc >= h;

		// Le contour ne passe pas par ce triangle
		if (sa == sb && sb == sc)
			continue;

		QVector<Vector2> intersections;
		QVector<int> facingTriangles;

		if (sa != sb)
		{
			double t = (h - Math::Min(va, vb)) / Math::Abs(va - vb);
			Vector2 p = Vector2::Lerp(pa, pb, va < vb ? t : 1 - t);
			intersections.append(p);
			facingTriangles.append(topologyExt->TriangleFacingVertex(ti, 2));
		}

		if (sa != sc)
		{
			double t = (h - Math::Min(va, vc)) / Math::Abs(va - vc);
			Vector2 p = Vector2::Lerp(pa, pc, va < vc ? t : 1 - t);
			intersections.append(p);
			facingTriangles.append(topologyExt->TriangleFacingVertex(ti, 1));
		}

		if (sb != sc)
		{
			double t = (h - Math::Min(vb, vc)) / Math::Abs(vb - vc);
			Vector2 p = Vector2::Lerp(pb, pc, vb < vc ? t : 1 - t);
			intersections.append(p);
			facingTriangles.append(topologyExt->TriangleFacingVertex(ti, 0));
		}

		if (intersections.size() != 2)
		{
			qDebug() << "error: should not happend" << sa << sb << sc;
			exit(0);
		}

		int id1 = ti + facingTriangles[0] * m;
		int id2 = ti + facingTriangles[1] * m;
		int inv_id1 = facingTriangles[0] + ti * m;
		int inv_id2 = facingTriangles[1] + ti * m;

		int index1;
		int index2;

		if (edgeToPoint.contains(inv_id1))
		{
			index1 = edgeToPoint[inv_id1];
		}
		else
		{
			edgeToPoint[id1] = points.size();
			index1 = points.size();
			points.append(intersections[0]);
		}

		if (edgeToPoint.contains(inv_id2))
		{
			index2 = edgeToPoint[inv_id2];
		}
		else
		{
			edgeToPoint[id2] = points.size();
			index2 = points.size();
			points.append(intersections[1]);
		}

		indices.append(index1);
		indices.append(index2);
	}

	SegmentSet2 ss(points, indices);
	return ss.GetPolygons();
}

/*
 * Debug des valeurs du graphe
 */
QGraphicsScene* GraphPoisson::ToScene(const DisplayOptions& opt) const
{
	QGraphicsScene* scene = new QGraphicsScene;

	if (Size() == 0)
		return scene;

	double r = radius;
	double min, max;
	GetRange(min, max);

	if (opt.minVal != -Math::Infinity) min = opt.minVal;
	if (opt.maxVal != Math::Infinity) max = opt.maxVal;

	if (opt.displayExternalNodes)
	{
		for (int i = 0; i < topologyExt->VertexSize(); ++i)
		{
			if (extToTopo[i] == -1)
			{
				Vector2 p = topologyExt->Vertex(i);

				for (int ni : topologyExt->VertexNeighboursVertices(i))
				{
					Vector2 p2 = topologyExt->Vertex(ni);
					Vector2 d = Normalized(p2 - p);
					Vector2 p1 = p + d * r;
					p2 = p2 - d * r;
					scene->addLine(p1[0], p1[1], p2[0], p2[1], QPen(QBrush(opt.edgeColor), r * opt.edgeRatio));
				}

				scene->addEllipse(p[0] - r, p[1] - r, 2 * r, 2 * r, QPen(QBrush(opt.outlineColor), r * opt.externalOutlineRatio), QBrush(opt.externalNodeColor));
			}
		}
	}

	auto inColoredRange = [this, &opt](int i) { return At(i) <= opt.maxColoredVal && At(i) >= opt.minColoredVal; };
	auto inDisplayedRange = [this, &opt](int i) { return At(i) <= opt.maxDisplayedVal && At(i) >= opt.minDisplayedVal; };

	for (int i = 0; i < Size(); ++i)
	{
		Vector2 p1 = Position(i);
		for (int n : Neighbours(i))
		{
			Vector2 p2 = Position(n);
			if (inDisplayedRange(i) && inDisplayedRange(n))
				scene->addLine(p1[0], p1[1], p2[0], p2[1], QPen(QBrush(opt.edgeColor), r * opt.edgeRatio));
		}
	}

	for (int i = 0; i < Size(); ++i)
	{
		QColor col;
		double ratio;
		if (!inDisplayedRange(i))
		{
			continue;
		}
		else if (!inColoredRange(i))
		{
			col = opt.uncoloredNodeColor;
			ratio = opt.uncoloredOutlineRatio;
		}
		else
		{
			double k = Math::Abs(max - min) < 0.0001 ? 0 : (At(i) - min) / (max - min);
			col = opt.palette->GetColor(k).GetQt();
			ratio = opt.outlineRatio;

			for (int n : Neighbours(i))
				if (!inColoredRange(n))
					ratio = opt.inRangeBorderOutlineRatio;

			if (opt.displayMaxValNodes && At(i) == opt.maxColoredVal)
			{
				if (opt.displayMaxSpecificColor)
					col = opt.selectedNodeColor;
				ratio = opt.selectedOutlineRatio;
			}
		}
		Vector2 p = Position(i);
		if (opt.displayBorder && IsBorder(i))
			scene->addEllipse(p[0] - r, p[1] - r, 2 * r, 2 * r, QPen(QBrush(opt.outlineColor), r * opt.borderOutlineRatio), QBrush(opt.borderNodeColor));
		else
			scene->addEllipse(p[0] - r, p[1] - r, 2 * r, 2 * r, QPen(QBrush(opt.outlineColor), r * ratio), QBrush(col));
	}

	return scene;
}

// Renvoie vraie si l'indice de la topologie extérieur correspond à un sommet dans la box du masque où à l'extérieur
// En gros renvoie vraie si on est dans la mer
bool GraphPoisson::ExteriorPointInsideMask(int evi) const
{
	return mask.GetBox().Inside(topologyExt->Vertex(evi));
}
