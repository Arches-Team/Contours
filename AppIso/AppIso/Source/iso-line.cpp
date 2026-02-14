#include "iso-line.h"
#include "draw.h"
#include "curvepoint.h"
#include "displacement-function.h"

using namespace std;

/*!
\brief Empty Constructor.
*/
IsoLinePoly::IsoLinePoly() : Polygon2(), h(0)
{
	//ChangeOrder(true);
}

IsoLinePoly::IsoLinePoly(const QVector<Vector2>& pts, double h) : Polygon2(pts), h(h)
{
}

/*!
\brief Constructor.

\param h Height of the iso.
\param poly The polygon corresponding to the border of the iso.
*/
IsoLinePoly::IsoLinePoly(const Polygon2& poly, double h) : Polygon2(poly), h(h)
{
	//ChangeOrder(true);
}

/*!
\brief Return the height of the iso.
*/
double IsoLinePoly::H() const
{
	return h;
}

/*!
\brief Change the height of the iso.
*/
void IsoLinePoly::SetH(double newH)
{
	h = newH;
}

/*!
\brief Remove every segment with length less than eps
*/
void IsoLinePoly::Epurate(double eps)
{
	QVector<Vector2> epuratePoly;
	epuratePoly.append(q.at(0));

	// On n'ajoute que si le segment est de taille suffisante
	for (int i = 1; i < q.size(); ++i)
	{
		if (Norm(q.at(i) - epuratePoly.constLast()) > eps)
		{
			epuratePoly.append(q.at(i));
		}
	}

	// On vérifie la relation entre le premier et le dernier point
	// On continue tant que le dernier segment est trop gros (car rien n'affirme qu'il ne sera pas trop grand une fois le dernier point supprimé)
	while (!epuratePoly.isEmpty() && Norm(epuratePoly.constFirst() - epuratePoly.constLast()) < eps)
	{
		epuratePoly.removeLast();
	}

	q = epuratePoly;
}

/*!
\brief Scale the polygon from eps following the normal
*/
void IsoLinePoly::Extend(double eps)
{
	QVector<Vector2> newQ;
	newQ.reserve(q.size());

	for (int i = 0; i < q.size(); ++i)
	{
		newQ.append(q[i] + eps * VertexNormal(i));
	}

	q = newQ;
}

/*!
\brief Compute the signed area of the polygon.
If the area is negative, it means that points are given in the wrong order (CW order instead of CCW)
*/
double IsoLinePoly::SignedArea() const
{
	double a = 0.0;

	for (int i = 0; i < q.size(); i++)
	{
		const Vector2& pa = q[i];
		const Vector2& pb = q[(i + 1) % q.size()];

		a += pa[0] * pb[1] - pb[0] * pa[1];
	}

	return a;
}

/*!
\brief Return true if the polygon points are given in trigonomic order
*/
bool IsoLinePoly::IsTrigo() const
{
	return SignedArea() > 0;
}

/*!
\brief Return true if two segments of the polygon intersect each other. A polygon with flat borders will return true to this
*/
bool IsoLinePoly::IsAutoIntersecting() const
{
	int n = q.size();
	for (int i = 0; i < n; i++)
	{
		Segment2 s1(q.at(i), q.at((i + 1) % n));

		for (int j = 0; j < n; j++)
		{
			if (i != j && i != (j + 1) % n && i != (j + n - 1) % n)
			{
				Segment2 s2(q.at(j), q.at((j + 1) % n));
				if (s1.Intersect(s2))
				{
					return true;
				}
			}
		}
	}
	return false;
}

/*!
\brief Change from CW to CCW and vice-versa
*/
void IsoLinePoly::ChangeOrder(bool trigo)
{
	if (IsTrigo() != trigo)
		std::reverse(q.begin(), q.end());
}

/*!
\brief Add points all along the polygon so the distance between two points is less than distMax
*/
IsoLinePoly IsoLinePoly::Resample(double distMax) const
{
	QVector<Vector2> pts;

	int n = q.size();
	for (int i = 0; i < n; ++i)
	{
		Vector2 dir = q[(i + 1) % n] - q[i];
		double dist = Norm(dir);

		// ajout premier point du segment
		pts.append(q[i]);

		// ajout points intermédiaires si besoin
		if (dist > distMax)
		{
			// nombre de points qu'il devrait y avoir entre 
			int n = ceil(dist / distMax);
			double step = 1. / n;
			for (int j = 1; j < n; ++j)
			{
				pts.append(q[i] + dir * (j * step));
			}
		}
	}

	return IsoLinePoly(Polygon2(pts), H());
}

/*!
\brief Add points all along the polygon based on Cardinal Spline

\param distMax The maximal distance between two points (not exactly respected since we go from straight lines to curves)
\param card The cardinal spline parameter
*/
IsoLinePoly IsoLinePoly::ResampleSpline(double distMax, double card) const
{
	QVector<Vector2> pts;

	int n = q.size();
	for (int i = 0; i < n; ++i)
	{
		Vector2 a = q[(i + n - 1) % n];
		Vector2 b = q[i];
		Vector2 c = q[(i + 1) % n];
		Vector2 d = q[(i + 2) % n];

		Vector2 t1 = (1 - card) * (c - a);
		Vector2 t2 = (1 - card) * (d - b);

		double dist = Norm(c - b);

		// ajout premier point du segment
		pts.append(q[i]);

		// ajout points intermédiaires si besoin
		if (dist > distMax)
		{
			// Define the spline between points i and i + 1
			CubicCurve2 spline = CubicCurve2::Hermite(b, c, t1, t2);

			// nombre de points qu'il devrait y avoir entre 
			int n = ceil(dist / distMax);
			double step = 1. / n;
			for (int j = 1; j < n; ++j)
			{
				pts.append(spline(j * step));
			}
		}
	}

	return IsoLinePoly(Polygon2(pts), H());
}

/*!
\brief Smooth the polygon by doing an weighted average with neighbours

\param kmax The number of neighbours to compute the average
\param delta The weight of the neighbours compared to the current point
\param niter The number of smooth iterations
*/
IsoLinePoly IsoLinePoly::Smooth2(int kmax, double delta, int niter) const
{
	int n = Size();
	QVector<Vector2> pts = Vertices();
	QVector<Vector2> nextpts = pts;
	for (int _ = 0; _ < niter; ++_)
	{
		for (int i = 0; i < n; ++i)
		{
			Vector2 q(0, 0);
			for (int k = -kmax; k <= kmax; ++k)
			{
				if (k != 0)
					q += pts[(i + n + k) % n];
			}
			q /= (2 * kmax);
			nextpts[i] = (1 - delta) * pts[i] + delta * q;
		}
		pts = nextpts;
	}
	return IsoLinePoly(Polygon2(pts), h);
}

/*!
\brief Return the position of a specific point of the polygon after smoothing it - doing a weighted average with neighbours
\warn The point is not changed in the polygon, you should do it manually if you want to
\warn The smoothing is not with specific neighbours but within a specific distance, as opposition with IsoLinePoly::Smooth

\param i index of the point in the polygon
\param d distance max to look for neighbours for the smoothing
\param delta The weight of the neighbours compared to the current point
*/
Vector2 IsoLinePoly::SmoothPoint(int i, double d, double delta) const
{
	int n = Size();
	// On fait un set pour éviter de devoir vérifier tous les indices
	// En vrai on peut faire plus efficacement et moins sale mais flemme
	QSet<int> neighs;

	// Right neighbours
	Vector2 previous = q[i];
	int ind = (i + 1) % n;
	double dist = 0;
	while (ind != i)
	{
		Vector2 neigh = q[ind];
		dist += Norm(previous - neigh);
		previous = neigh;
		if (dist <= d)
			neighs.insert(ind);
		else
			break;
		ind = (ind + 1) % n;
	}

	// Left neighbours
	previous = q[i];
	ind = (i - 1 + n) % n;
	dist = 0;
	while (ind != i)
	{
		Vector2 neigh = q[ind];
		dist += Norm(previous - neigh);
		previous = neigh;
		if (dist <= d)
			neighs.insert(ind);
		else
			break;
		ind = (ind - 1 + n) % n;
	}

	if (neighs.size() == 0)
		return q[i];

	Vector2 new_p(0, 0);
	for (int neigh : neighs)
	{
		new_p += q[neigh];
	}
	new_p /= neighs.size();
	new_p = (1 - delta) * q[i] + delta * new_p;
	return new_p;
}

/*!
\brief Smooth the entire polygon

\param d distance max to look for neighbours for the smoothing
\param delta The weight of the neighbours compared to the current point
\param niter The number of smooth iterations
*/
IsoLinePoly IsoLinePoly::Smooth(double d, double delta, int niter) const
{
	int n = Size();
	IsoLinePoly ilp(Polygon2(q), h);
	QVector<Vector2> pts = q;
	for (int _ = 0; _ < niter; ++_)
	{
		for (int i = 0; i < n; ++i)
		{
			pts[i] = ilp.SmoothPoint(i, d, delta);
		}
		ilp.q = pts;
	}
	return ilp;
}

/*!
\brief Smooth the polygon within a specific circle only

\param c The circle
\param d distance max to look for neighbours for the smoothing
\param delta The weight of the neighbours compared to the current point
\param niter The number of smooth iterations
*/
IsoLinePoly IsoLinePoly::SmoothInsideCircle(const Circle2& c, double d, double delta, int niter) const
{
	int n = Size();
	QVector<Vector2> pts = Vertices();
	QVector<Vector2> nextpts = pts;
	for (int _ = 0; _ < niter; ++_)
	{
		for (int i = 0; i < n; ++i)
		{
			if (c.Inside(q[i]))
			{
				nextpts[i] = SmoothPoint(i, d, delta);
			}
		}
		pts = nextpts;
	}
	return IsoLinePoly(Polygon2(pts), h);
}

/*!
\brief Move the points of the polygon within a specific circle towards its center

\param c The circle
\param r the ratio of the displacement: 1 = go to the circle's center, -1 = go away from the circle's center
*/
IsoLinePoly IsoLinePoly::LocalDisplacement(const Circle2& c, double r) const
{
	Vector2 center = c.Center();
	int n = Size();
	QVector<Vector2> pts = Vertices();
	for (int i = 0; i < n; ++i)
	{
		if (c.Inside(q[i]))
		{
			Vector2 dir = center - q[i];
			pts[i] += dir * r;
		}
	}
	return IsoLinePoly(Polygon2(pts), h);
}

/*!
\brief Move the points of the polygon within a specific curve towards its center

\param c The curve
\param r The radius of the impact of the curve
\param strength the strength of the displacement: -1 = go to the curve, 1 = go away from the curve
\param k the power of the displacement function
*/
IsoLinePoly IsoLinePoly::DisplacementTowardCurve(const PointCurve2& c, double r, double strength, double k) const
{
	int n = Size();
	QVector<Vector2> pts = Vertices();
	
	DisplacementFunction1 delta(k);
	double minderi = delta.MinDerivative();

	for (int i = 0; i < n; ++i)
	{
		int k;
		double u;
		double d = c.R(q[i], u, k);

		if (d < r*r)
		{
			Vector2 p1 = c.At(k);
			Vector2 p2 = c.At(k + 1);
			Segment2 edge(p1, p2);
			edge.R(q[i], u);

			Vector2 a = p1 + (p2 - p1) * u;
			Vector2 b = a + r * Normalized(q[i] - a);
			double u = Norm(q[i] - a) / r;
			Vector2 displacement = strength * delta(u) / minderi * (b - a);
			pts[i] += displacement;
		}
	}
	return IsoLinePoly(Polygon2(pts), h);
}

/*!
\brief Returns if the given polygon intersects with this polygon, and if not, which one contains the other.

\todo Pass this function into Polygon2 class
	  Actuellement en O(n^2)
	  Peut être amélioré avec l'algorithme de Bentley-Ottmann en O((n + m)log(n + m))

\param p The polygon for comparison.
*/
IsoLinePoly::IntersectType IsoLinePoly::RelationWith(const Polygon2& p) const
{
	for (int i = 0; i < q.size(); i++)
	{
		Segment2 ab(q.at(i), q.at((i + 1) % q.size()));
		if (p.Intersect(ab))
		{
			return IntersectType::INTERSECT;
		}
	}

	// Si on n'a aucune intersection avec un segment, on a trois possibilités
	// 1. On contient complètement l'autre polygone
	if (Inside(p.Vertex(0)))
	{
		return IntersectType::CONTAINS;
	}

	// 2. On est complètement contenu dans l'autre polygone
	if (p.Inside(q.at(0)))
	{
		return IntersectType::INSIDE;
	}

	// 3.Les deux polygones sont complètement disjoints
	return IntersectType::INDEPENDENT;
}

/*
\brief Compute the normal of one vertex of a polygon according to its neighbours.

Warning!! The vertices of the polygon need to be given in trigonometric order.

\return the direction of the normal.
*/
Vector2 IsoLinePoly::VertexNormal(int i) const
{
	int n = Size();
	Vector2 a = q.at((i - 1 + n) % n);
	Vector2 b = q.at(i);
	Vector2 c = q.at((i + 1) % n);

	Vector2 v1 = Normalized(a - b);
	Vector2 v2 = Normalized(c - b);

	Vector2 bisector = v1 + v2;

	// Les points sont alignés, donc la bissectrice est indéfinie
	// mais dans notre cas, on veut juste le vecteur à 90 deg CCW (trigonometric order) par rapport au Vector2 v1
	if (Norm(bisector) < 1e-12)
	{
		return Vector2(-v1[1], v1[0]);
	}

	// La bissectrice pointe toujours vers l'intérieur de l'angle
	// Donc si on a un angle obtu (produit vectoriel négatif) on envoie l'inverse
	if (v1 / v2 < 0)
		return -Normalized(bisector);
	return Normalized(bisector);
}

/*!
\brief Compute a simple polygon triangulation using a ear clipping method.

Warning!! The vertices of the polygon need to be given in trigonometric order.
Moreover, this method doesn't work with auto-intersecting polygons (including polygons where only one point lies on one other segment)

\return Indices triplet which define triangles.
*/
QVector<int> IsoLinePoly::EarClip2() const
{
	if (q.size() < 3)
		return QVector<int>();

	int size = q.size();

	QStack<int> vertices; // Is it better to use a stack or a queue?
	QVector<int> indices(size * 3);
	vertices.reserve(size * 3);
	QVector<int> prev(size, -1);
	QVector<int> next(size, -1);

	for (int i = 0; i < size; i++)
	{
		int ia = i;
		int ib = (i + 1) % size;
		int ic = (i + 2) % size;

		vertices.push(ia);
		vertices.push(ib);
		vertices.push(ic);

		prev[ib] = ia;
		next[ib] = ic;
	}

	// While true, but we stop if the queue is empty (for auto-intersecting polygons for example)
	while (vertices.size() >= 3)
	{
		// triangle ia -- ib -- ic
		int ic = vertices.pop();
		int ib = vertices.pop();
		int ia = vertices.pop();

		// This triangle is not possible anymore because ia (or ic) is already in one triangle of the triangulation
		if (prev[ib] != ia || next[ib] != ic) continue;

		// If ib is above the segment ia -- ic, then the triangle is not inside but outside the polygon
		// This is because we know the polygon points are given in trigonometric order
		// If we have a flat triangle, we remove it from the possibilities
		// At the end, if we only have flat triangles, then some points won't be attached to the triangulation
		// TODO: maybe do something more robust than == 0.0 for flat triangles
		double cross = Normalized(q[ib] - q[ia]) / Normalized(q[ic] - q[ia]);
		if (cross <= 0.0) continue;

		// This is the O(n) part of the loop: to be sure the triangle is correct no points of the polygon should be inside it
		bool inside = false;
		Triangle2 t(q[ia], q[ib], q[ic]);

		for (int i = 0; i < size; ++i)
		{
			if (i != ia && i != ib && i != ic && t.Inside(q[i]))
			{
				inside = true;
				break;
			}
		}

		if (inside) continue;

		// Creation of the triangle
		indices.append(ia);
		indices.append(ib);
		indices.append(ic);

		// Modification of pointers
		prev[ic] = ia;
		next[ia] = ic;

		// When we have a polygon with only 2 vertices, triangulation is over
		if (prev[ia] == next[ia]) break;

		// There are two new possible triangles
		vertices.push(prev[ia]);
		vertices.push(ia);
		vertices.push(ic);

		vertices.push(ia);
		vertices.push(ic);
		vertices.push(next[ic]);
	}

	return indices;
}



IsoLines::IsoLines(const QVector<Polygon2>& polys, double hmin, double hmax, bool simple) : simple(simple)
{
	for (const Polygon2& p: polys)
	{
		if (!Append(IsoLinePoly(p)))
		{
			cerr << "A polygon has been discarded from the set, check for collisions." << endl;
		}
	}

	ChangeHeightsMinMaxEndoreic(hmin, hmax);

	if (!simple && !WellFormed())
		cerr << "ill-formed polygons" << endl;
}

IsoLines::IsoLines(const Polygons2& polys, double hmin, double hmax, bool simple) : simple(simple)
{
	for (int i = 0; i < polys.Size(); ++i)
	{
		if (!Append(IsoLinePoly(polys.At(i))))
		{
			cerr << "A polygon has been discarded from the set, check for collisions." << endl;
		}
	}
	
	ChangeHeightsMinMaxEndoreic(hmin, hmax);

	if (!simple && !WellFormed())
		cerr << "ill-formed polygons" << endl;
}

IsoLines::IsoLines(const QVector<IsoLinePoly>& ilps, bool simple) : simple(simple)
{
	for (const IsoLinePoly& iso : ilps)
	{
		if (!Append(iso))
		{
			cerr << "An iso has been discarded from the set, check for collisions." << endl;
		}
	}

	if (!simple && !WellFormed())
		cerr << "ill-formed isolines" << endl;
}

IsoLines::IsoLines(const ScalarField2& sf, const QSet<double>& heights, bool simple) : simple(simple)
{
	// Cas spécifique qui fait buguer `LineSegments()`
	if (sf.VertexSize() == 0)
		return;

	// On trie les valeurs pour que les isos soient mises (et Extend) dans le bon ordre
	QVector<double> values(heights.begin(), heights.end());
	std::sort(values.begin(), values.end());
	int n = values.size();

	// Le but de tout ce calcul est d'éviter d'avoir une hauteur qui soit exactement une valeur d'un pixel du champ scalaire.
	// Si c'est le cas, on risque d'avoir des segments qui se chevauchent et qui sont considérés comme s'auto-croisant, à cause des "crètes".
	// Ces crètes se forment lorsque 2 pixels voisins de meme valeur sont entourés par des valeurs + grandes ou + basses.
	// Donc on fait en sorte qu'aucune hauteur ne soit exactement la valeur d'un pixel pour éviter que ça arrive.
	QSet<double> specificHeights;
	for (int i = 0; i < sf.VertexSize(); ++i)
		specificHeights.insert(sf.at(i));

	double min, max;
	sf.GetRange(min, max);

	for (double& h : values)
		if (specificHeights.contains(h))
			h = h + (max - min) / (heights.size() * 100); // Attention, petit epsilon qui ne marche pas nécessairement avec tous les terrains, mais suffisant pour l'instant
	// Fin correction

	for (int i = 0; i < n; ++i)
	{
		double h = values[i];

		SegmentSet2 ss2 = sf.LineSegments(h, true);
		Polygons2 p2 = ss2.GetPolygons(true);
		for (int j = 0; j < p2.Size(); ++j)
		{
			IsoLinePoly ilp(p2.At(j), h);
			// Si on supprime pas les trop petit segments avant d'extend, le polygone risque de s'auto-croiser. Note que c'est suffisant pour mes tests, mais la longueur des segments ne garanti pas aucune auto-intersection de Extend
			ilp.Epurate(0.001);
			// On étend le long de la normal en fonction de la hauteur pour ne pas avoir de polygone ayant une arète en commun, ce qui est souvent le cas si on n'est pas sur une ile. Et les tests avec des polygones qui partagent un segment c'est relou.
			ilp.Extend(0.001 * (n - i) / (double)n);

			// On ne garde que les grosses isos
			if (ilp.Size() > 10)
			{
				if (!Append(ilp))
				{
					cerr << "An iso has been created from hf but has been discarded, not normal." << endl;
				}
			}
		}
	}

	if (!simple && !WellFormed())
		cerr << "ill-formed heightfield" << endl;
}

IsoLines::IsoLines(const ScalarField2& sf, int nbLevels, bool simple) : IsoLines(sf, [nbLevels, &sf]() {
		QSet<double> heights;
		double a, b;
		sf.GetRange(a, b);
		for (int i = 0; i < nbLevels; ++i)
		{
			double t = i / (double)nbLevels;
			double h = Math::Lerp(a, b, t);
			heights.insert(h);
		}
		return heights;
	}(), simple)
{

}

// Récupère les isos entre les valeurs de histo (utilisé pour donner les nodes du graphe poisson)
// Utilise le fait que le graph poisson a été assigné via une classe IsoVectoGeneration
// On récupère les isos au hauteur (h[i] + h[i+1]) / 2 pour éviter les iso "crètes" (voir article)
IsoLines::IsoLines(const GraphPoisson& gp, const HistogramD& histo, bool simple) : simple(simple)
{
	QVector<double> heights;
	heights.append(histo.GetKey(0) + (histo.GetKey(0) - histo.GetKey(1)) / 2);
	for (int i = 0; i < histo.Size() - 1; ++i)
	{
		heights.append((histo.GetKey(i) + histo.GetKey(i+1)) / 2);
	}
	double a, b;
	gp.GetRange(a, b);

	for (double h : heights)
	{
		Polygons2 polys = gp.ContourLines(h);
		for (int j = 0; j < polys.Size(); ++j)
		{
			IsoLinePoly ilp(polys.At(j), h);
			if (!Append(ilp))
			{
				cerr << "An iso has been created from graph poisson but has been discarded, not normal." << endl;
			}
		}

		if (polys.Size() == 0)
		{
			qDebug() << "There is no isoline extracted for height" << h;
		}
	}

	if (!simple && !WellFormed())
		cerr << "ill-formed graphpoisson" << endl;
}

Box2 IsoLines::GetBox() const
{
	Box2 b = Box2::Null;
	for (const IsoLinePoly& ilp : isos)
	{
		b = Box2(b, ilp.GetBox());
	}
	return b;
}

void IsoLines::GetRange(double& hmin, double& hmax) const
{
	hmin = isos.first().H();
	hmax = isos.last().H();

	for (const IsoLinePoly& ilp : isos)
	{
		hmin = Math::Min(hmin, ilp.H());
		hmax = Math::Max(hmax, ilp.H());
	}
}

/*!
\brief Compute the average height between two consecutive iso heights
*/
double IsoLines::AverageHeightBetweenIso() const
{
	int levels = Levels();
	double a, b;
	GetRange(a, b);
	return (b - a) / levels;
}

/*!
\brief Return the height inside iso i (the isos needs to be well-formed)
This is not as simple as calling iso.H() because sometimes the iso refer to a growing iso and sometimes to a shrinking iso

Be carefull, sometimes, an iso can have no height inside. This is the case for internal border, check InternalBorder() before calling this since we don't know the height outside the set.
*/
double IsoLines::HeightInside(int i) const
{
	double h = isos[i].H();
	// When simple, we cannot have this information, avoid calling this method with simple
	if (simple || isGrowing(i))
		return h;

	QSet<double> heights;
	for (int c : children[i])
		heights.insert(isos[c].H());

	heights.remove(h);

	// We have the height of a child iso which is lower since we are shrinking, so we have the exact good height
	if (!heights.isEmpty())
		return *heights.begin();

	// Otherwise, we cannot know the depth, so we adjust considering the difference between the parent and us.
	int p = parents[i];
	// We can have multiple isos at same height so it does not help
	while (p != -1 && isos[p].H() == h)
		p = parents[p];

	// Impossible to know, just give up (this is the case for InternalBorders
	if (p == -1)
		return h;
	return h - Math::Abs(h - isos[p].H());
}

/*
 * Return true if the isoline correspond to an internal border, ie a border of the terrain inside
 * (equivalent to an interior sea for example if we consider the sea level as the exterior) 
 */
bool IsoLines::InternalBorder(int i) const
{
	double a, b;
	GetRange(a, b);
	return !isGrowing(i) && isos[i].H() == a;
}


/*
 * Return a copy of the isolines, with the center of them in (0, 0)
 */
IsoLines IsoLines::Centered() const
{
	IsoLines cp = *this;
	Box2 b = GetBox();
	Vector2 c = b.Center();

	for (int i = 0; i < Size(); ++i)
	{
		cp.isos[i] = isos[i].Translated(-c);
	}
	return cp;
}

/*!
\brief Compute the size of the iso tree
\warn No meaning when simple = true
*/
int IsoLines::Levels() const
{
	int level = 0;
	for (int c : roots)
	{
		level = Math::Max(level, Levels(c));
	}
	return level;
}

/*!
\brief Compute the size of the iso tree starting from the iso i
\warn No meaning when simple = true
*/
int IsoLines::Levels(int i) const
{
	int level = 1;
	for (int c : children[i])
	{
		level = Math::Max(level, Levels(c) + 1);
	}
	return level;
}

/*!
\brief Returns true if the set is well-formed (ie if all neighbouring zones have the same height)
\warn No meaning when simple = true

To be well-formed, all isos in the root should all go up by convention.

If this return false, there is no real meaning into the set of isolines, and you should consider it as if simple = true.
*/
bool IsoLines::WellFormed() const
{
	if (roots.isEmpty())
		return true;

	double h = isos[*roots.begin()].H();
	for (int c : roots)
		if (h != isos[c].H())
			return false;

	for (int c : roots)
		if (!WellFormed(c, true))
			return false;

	return true;
}

/*!
\brief Returns true if the iso i and inside is well formed, knowing that the heights grow (if up == true) or shring (if up == false) 
\warn No meaning when simple = true

To be well-formed, an iso at a certain height h can only have
- interior isos at height h (in this case, the next iso change from grow to shrink and vice-versa)
- interior isos with all the same height h2. h2 needs to be > h if up = true, and needs to be < h if up = false
*/
bool IsoLines::WellFormed(int i, bool up) const
{
	if (children[i].isEmpty())
		return true;

	double h = isos[i].H();
	QSet<double> heights;
	for (int c : children[i])
		heights.insert(isos[c].H());

	if (heights.contains(h))
		heights.remove(h);
	if (heights.size() > 1)
		return false;
	if (heights.size() == 1 && !(*heights.begin() > h == up)) // if the only other height as not the good direction
		return false;

	for (int c : children[i])
		if (!WellFormed(c, isos[c].H() == h ? !up : up))
			return false;

	return true;
}

/*!
\brief Returns if the set of iso endoreic or not, you need to check WellFormed() before!
\warn No meaning when simple = true
*/
bool IsoLines::isEndoreic() const
{
	for (int c : roots)
		if (isEndoreic(c))
			return true;

	return false;
}

/*!
\brief Returns if the iso i and inside is endoreic or not, you need to check WellFormed(i) or WellFormed() before!
\warn No meaning when simple = true
*/
bool IsoLines::isEndoreic(int i) const
{
	if (children[i].isEmpty())
		return false;

	QSet<double> heights;
	for (int c : children[i])
		heights.insert(isos[c].H());

	if (heights.size() > 1)
		return true;

	if (*heights.begin() <= isos[i].H())
		return true;

	for (int c : children[i])
		if (isEndoreic(c))
			return true;

	return false;
}

/*!
\brief Returns if the iso i is the border of a zone going up (true) or goind down (false), you need to check WellFormed(i) or WellFormed() before!
\warn No meaning when simple = true
*/
bool IsoLines::isGrowing(int i) const
{
	int p = parents[i];
	if (p == -1) // all border isos are going up by convention
		return true;
	if (isos[p].H() < isos[i].H())
		return true;
	if (isos[p].H() > isos[i].H())
		return false;
	else // we change sign when we have the same height than our parent because it designate a bump or a valley.
		return !isGrowing(p);
}

/*!
\brief Change heights of the entire set between min and max
*/
void IsoLines::ChangeHeightsMinMax(double min, double max)
{
	double hmin, hmax;
	GetRange(hmin, hmax);
	double factor = Math::Abs(hmax - hmin) > 0.000001 ? hmax - hmin : 1;
	double nfactor = Math::Abs(max - min) > 0.000001 ? max - min : 0;
	for (int i = 0; i < Size(); ++i)
	{
		double h = ((isos[i].H() - hmin) / factor) * nfactor + min;
		isos[i].SetH(h);
	}
}

/*!
\brief Change heights of the entire set between min and max but do not conserve non-endoreic area
*/
void IsoLines::ChangeHeightsMinMaxEndoreic(double min, double max)
{
	int levels = Levels();
	double baseH = min;
	double stepH = levels <= 1 ? 0 : (max - min) / (levels - 1);

	ChangeHeights(baseH, stepH);
}

/*!
\brief Change heights of the entire set starting from baseH and increasing of stepH at each step
\warn Do not work when simple = true

We consider here that the set is non endoreic, because otherwise the method would be much harder to compute.
(At each recursion we should know if we need to move up or down in heights. Non endoreic = move up every time.)
*/
void IsoLines::ChangeHeights(double baseH, double stepH)
{
	for (int c : roots)
	{
		ChangeHeights(c, baseH, stepH);
	}
}

/*!
\brief Change heights of the iso i and inside starting from baseH and increasing of stepH at each step
\warn Do not work when simple = true
*/
void IsoLines::ChangeHeights(int i, double baseH, double stepH)
{
	isos[i].SetH(baseH);
	for (int c : children[i])
	{
		ChangeHeights(c, baseH + stepH, stepH);
	}
}

void IsoLines::Smooth(double d, double delta, int niter)
{
	for (int i = 0; i < Size(); ++i)
	{
		isos[i] = isos[i].Smooth(d, delta, niter);
	}
}

// Only smooth bigger isos, avoid to reduce small ones to lines or points
void IsoLines::SmoothBig(int minSize, double d, double delta, int niter)
{
	for (int i = 0; i < Size(); ++i)
	{
		if (isos[i].Size() > minSize)
		{
			isos[i] = isos[i].Smooth(d, delta, niter);
		}
	}
}

void IsoLines::Resample(double distMax)
{
	for (int i = 0; i < Size(); ++i)
	{
		isos[i] = isos[i].Resample(distMax);
	}
}

void IsoLines::ResampleSpline(double distMax, double card)
{
	for (int i = 0; i < Size(); ++i)
	{
		isos[i] = isos[i].ResampleSpline(distMax, card);
	}
}

/*!
\brief return yes if the point is inside the set of isolines
\warn Do not work when simple = true
*/
bool IsoLines::Inside(const Vector2& p) const
{
	// We only check the roots isolines since every other are contained inside those one
	for (int i : Roots())
		if (isos[i].Inside(p))
			return true;
	return false;
}

/*!
\brief Compute the parent isoline of a point in space
\warn Do not work when simple = true

\param p	the point
\return		the parent index or -1 if the point is outside the isolines
*/
int IsoLines::ComputeParentIso(const Vector2& p) const
{
	int parent = -1;

	QSet<int> toCheck = roots;

	while (!toCheck.isEmpty())
	{
		int i = *toCheck.begin();
		toCheck.remove(i);

		const IsoLinePoly& iso = At(i);
		bool in = iso.Inside(p);

		if (in)
		{
			parent = i;
			toCheck = Children(i);
		}
	}

	return parent;
}

/*!
\brief Compute the parent isoline of a polygon
\warn Do not work when simple = true

\param p	  (in)	the polygon (which should not be a polygon from the current set)
\param parent (out)	index of the parent iso (i.e. the closest iso which contains the polygon), -1 if it does not exist
\return false		if the polygon is crossing one iso (and thus has no parent)
*/
bool IsoLines::ComputeParentIso(const Polygon2& p, int& parent) const
{
	parent = -1;

	QSet<int> toCheck = roots;

	while (!toCheck.isEmpty())
	{
		int i = *toCheck.begin();
		toCheck.remove(i);

		const IsoLinePoly& iso = At(i);
		IsoLinePoly::IntersectType t = iso.RelationWith(p);

		if (t == IsoLinePoly::INTERSECT)
		{
			return false;
		}
		else if (t == IsoLinePoly::CONTAINS)
		{
			parent = i;
			toCheck = Children(i);
		}
	}

	return true;
}

/*!
\brief Add a polygon as an isoline and automatically set its heights according to the neighbourhood
\warn When simple = true, the polygon value is the base value (which correspond to the value given for the first polygon)

TODO:	Maybe the way we handle isolines is not correct, maybe we should consider a min and max height and stick to it
		Because when we add an isoline between two isolines, the given height is strange since it should be the middle, not the next one
		But if we give the middle, then the difference of heights are not the same anymore
		Moreover, when we add an external isoline, it will have the min height, which is the same height than its parent, which is weird
		Maybe adding an isoline should change the height of every current isoline so it is distributed between min_height and max_height
		Problem to consider endoreic areas, maybe we should have a changeGrowth array with true or false value: true means that the heights
		change according to the previous. Iso 0 should grow, then for example:
			1 - continue
			2 - continue
			3 - change grow: same height than previous (now we shrink)
			4 - continue: shrink
			5 - change grow: same heigh than previous (now we grow)
			6 - continue: grow etc...
		The problem is that we should never have two changes of direction in a row (or maybe this is possible if we have waves)
*/
bool IsoLines::Append(const Polygon2& p, double baseValue)
{
	if (simple || Size() == 0)
		return Append(IsoLinePoly(p, baseValue));

	double hmin, hmax;
	GetRange(hmin, hmax);
	int parent;

	// Return false if intersection
	if (!ComputeParentIso(p, parent))
		return false;

	if (parent == -1)
		return Append(IsoLinePoly(p, hmin));

	QVector<double> heights = SortedHeights();
	int ind = 0;
	for (int i = 0; i < heights.size(); ++i)
	{
		if (Math::Abs(isos[parent].H() - heights[i]) < 0.000001)
		{
			ind = i;
			break;
		}
	}

	if (isGrowing(parent))
	{
		if (ind < heights.size() - 1)
		{
			return Append(IsoLinePoly(p, heights[ind + 1]));
		}
		return Append(IsoLinePoly(p, heights[ind]));
	}
	return Append(IsoLinePoly(p, heights[ind]));
}

/*!
\brief Add an isoline to the set only if it fits with other isos (= not intersecting)
\warn When simple = true, the isolines is always appended

/!\ Carefull, the new set can be ill-formed. You need to check WellFormed() to be sure of it
We do not check if the set is well-formed at each Append since multiple append may be sometimes necessary to add isos

\return true if the isoline has been appended.
*/
bool IsoLines::Append(const IsoLinePoly& ilp)
{
	// We only want non intersecting polygon for isolines
	// /!\ QUADRATIC, be careful with big polygons. TODO: improve this algo (using a sweeping line?)
	if (ilp.IsAutoIntersecting())
		return false;

	if (simple)
	{
		isos.append(ilp);
		children.append(QSet<int>());
		parents.append(-1);
		return true;
	}

	int parent;

	// Return false if intersection
	if (!ComputeParentIso(ilp, parent))
		return false;

	int ind = isos.size(); // index of the new isoline
	
	// New isoline outside (no parent)
	if (parent == -1)
	{
		QSet<int> newChildren = QSet<int>();
		QSet<int> newRoots = { ind };

		// Change the current roots
		for (int i : roots)
		{
			if (isos[i].RelationWith(ilp) == IsoLinePoly::INSIDE)
			{
				newChildren.insert(i);
				parents[i] = ind;
			}
			// not inside => independent
			else
			{
				newRoots.insert(i);
			}
		}

		isos.append(ilp);
		parents.append(-1);
		children.append(newChildren);
		roots = newRoots;
	}
	// new isoline inside another but with no siblings
	else if (children[parent].isEmpty())
	{
		isos.append(ilp);
		parents.append(parent);
		children.append(QSet<int>());
		children[parent].insert(ind);
	}
	// new isoline with siblings
	else
	{
		QSet<int> newChildren = QSet<int>();
		QSet<int> newParentChildren = { ind };

		// Check which sibling is inside the new one
		for (int i : Children(parent))
		{
			if (isos[i].RelationWith(ilp) == IsoLinePoly::INSIDE)
			{
				newChildren.insert(i);
				parents[i] = ind;
			}
			// not inside => independent
			else
			{
				newParentChildren.insert(i);
			}
		}

		isos.append(ilp);
		parents.append(parent);
		children.append(newChildren);
		children[parent] = newParentChildren;
	}

	return true;
}

/*!
\brief Remove the isoline of index ind, if recursive is set to true, remove every child of this iso
\warn recursive as no meaning when simple = true
*/
void IsoLines::Remove(int ind, bool recursive)
{
	Remove({ ind }, recursive);
}

void IsoLines::Remove(const QSet<int>& toRemove, bool recursive)
{
	if (!recursive)
		return Remove(toRemove);
	
	QSet<int> toRemoveRec;

	QQueue<int> queue;
	for (int ind : toRemove)
		queue.enqueue(ind);

	while (!queue.empty())
	{
		int cur = queue.dequeue();
		if (!toRemoveRec.contains(cur))
		{
			toRemoveRec.insert(cur);
			for (int child : Children(cur))
				queue.enqueue(child);
		}
	}

	Remove(toRemoveRec);
}

/*!
\brief Remove all isolines given by the set inds
\warn According to which indices are given, nothing prevent the new set to be well-formed!
*/
void IsoLines::Remove(const QSet<int>& inds)
{
	QVector<int> newIndices(Size(), -1);
	int new_size = 0;
	for (int i = 0; i < Size(); ++i)
		if (!inds.contains(i))
			newIndices[i] = new_size++;

	QVector<IsoLinePoly> new_isos;
	QVector<int> new_parents;
	QVector<QSet<int>> new_children;
	QSet<int> new_roots;

	new_isos.reserve(new_size);
	new_parents.reserve(new_size);
	new_children.reserve(new_size);

	for (int i = 0; i < Size(); ++i)
	{
		if (newIndices[i] != -1)
		{
			// we keep it simple if we are simple
			if (simple)
			{
				new_isos.append(isos[i]);
				new_children.append(QSet<int>());
				new_parents.append(-1);
			}
			else
			{
				new_isos.append(isos[i]);

				// First change the parent to be the first not removing parent (or no one = root)
				int parent = parents[i];
				while (parent != -1 && newIndices[parent] == -1)
					parent = parents[parent];
				if (parent != -1)
					new_parents.append(newIndices[parent]);
				else
					new_parents.append(-1);

				// For now only keep an empty chidlren list, we will populate it after all parents are created
				new_children.append(QSet<int>());
			}
		}
	}

	// populate roots and chidlren
	if (!simple)
	{
		qDebug() << new_size << new_parents.size() << new_children.size() << new_isos.size();
		for (int i = 0; i < new_size; ++i)
		{
			if (new_parents[i] == -1)
				new_roots.insert(i);
			else
				new_children[new_parents[i]].insert(i);
		}
	}

	isos = new_isos;
	parents = new_parents;
	children = new_children;
	roots = new_roots;
}

/*!
\brief Return the number of points in all isolines
*/
int IsoLines::VertexSize() const
{
	int n = 0;
	for (const IsoLinePoly& ilp : isos)
	{
		n += ilp.Size();
	}
	return n;
}

/*!
\brief Return the combined length of all isolines
*/
double IsoLines::TotalLength() const
{
	double l = 0;
	for (const IsoLinePoly& ilp : isos)
	{
		l += ilp.Length();
	}
	return l;
}

/*!
\brief Return a set containing all different heights of isolines
*/
QSet<double> IsoLines::Heights() const
{
	QSet<double> h;
	for (int i = 0; i < Size(); ++i)
		h.insert(isos[i].H());
	return h;
}

/*!
\brief Same as IsoLines::Heights but it is sorted
*/
QVector<double> IsoLines::SortedHeights() const
{
	QSet<double> h = Heights();
	QVector<double> heights(h.begin(), h.end());
	std::sort(heights.begin(), heights.end());
	return heights;
}

/*!
\brief Return the histogram corresponding to the area taken by each different height
*/
HistogramD IsoLines::HeightsHistogram() const
{
	QMap<double, double> height_to_areas;

	for (int i = 0; i < Size(); ++i)
	{
		double h = HeightInside(i);
		if (!height_to_areas.contains(h))
			height_to_areas[h] = 0;

		height_to_areas[h] += isos[i].Area();
		for (int c : children[i])
			height_to_areas[h] -= isos[c].Area();
	}

	QVector<double> heights = SortedHeights();
	QVector<double> areas(heights.size());
	for (int i = 0; i < heights.size(); ++i)
	{
		areas[i] = height_to_areas[heights[i]];
	}

	return HistogramD(heights, areas);
}

ScalarField2 IsoLines::GetMask(int w, int h) const
{
	return GetMask(GetBox(), w, h);
}

/*!
\brief Returns a scalarfield to serve as a mask for IsoGeneration the exterior as value 0, the interior is in the interval ]0,1]
\warn No meaning when simple = true
*/
ScalarField2 IsoLines::GetMask(const Box2& box, int w, int h) const
{
	double a, b;
	GetRange(a, b);
	double step = (b - a) / Levels();
	a -= step;

	ScalarField2 mask(box, w, h, 0);
	for (int x = 0; x < mask.GetSizeX(); ++x)
	{
		for (int y = 0; y < mask.GetSizeY(); ++y)
		{
			Vector2 c = mask.ArrayVertex(x, y);
			int iso = ComputeParentIso(c);
			if (iso != -1)
			{
				double h = (isos[iso].H() - a) / (b - a);
				mask(x, y) = h;
			}
		}
	}
	return mask;
}

#include <QtWidgets/QGraphicsPathItem>

QGraphicsScene* IsoLines::ToScene(const DisplayOptions& opt) const
{
	QGraphicsScene* scene = new QGraphicsScene;
	if (isos.isEmpty())
	{
		return scene;
	}

	double hmin;
	double hmax;
	GetRange(hmin, hmax);
	Box2 b = GetBox();

	//b.Draw(*scene, QPen(Qt::white), QBrush(Qt::white));

	// Pour donner une taille correcte au pinceau en fonction de la scene
	double l = Norm(b.Diagonal()) / 500 * opt.edge_size;

	// To remove internal borders
	QPainterPath outerPath;
	if (opt.fill)
	{
		for (int i = 0; i < Size(); ++i)
		{
			if (InternalBorder(i))
			{
				outerPath.addPolygon(isos[i].GetQt());
				outerPath.closeSubpath();
			}
		}
	}

	std::function<void(int, int)> draw = [&](int i, int level)
	{
		bool should_display = (opt.max_isos == -1 || level >= opt.min_isos) && (opt.max_isos == -1 || level <= opt.max_isos);

		if (should_display)
		{
			double t = (hmax - hmin) == 0 ? 1 : (isos[i].H() - hmin) / (hmax - hmin);
			double tin = (hmax - hmin) == 0 ? 1 : (HeightInside(i) - hmin) / (hmax - hmin);
			QColor c = opt.palette->GetColor(t).GetQt();
			QColor cin = opt.palette->GetColor(tin).GetQt();
			QPen pen(opt.fill ? Qt::black : c);
			pen.setWidth(l);
			// TODO: c'est bien pour zoomer, mais ça marche pas bien sur une scène a exporter
			//pen.setCosmetic(true);

			if (opt.fill)
			{
				QPainterPath innerPath;
				innerPath.addPolygon(isos[i].GetQt());
				innerPath.closeSubpath();
				QPainterPath finalPath = innerPath.subtracted(outerPath);
				auto* item = new QGraphicsPathItem(finalPath);
				item->setBrush(QBrush(cin));
				item->setPen(pen);
				scene->addItem(item);
			}
			else
			{
				isos[i].Draw(*scene, pen);
			}

			if (opt.arrow)
			{
				for (int j = 0; j < isos[i].Size(); ++j)
				{
					Segment2 s(isos[i].Vertex(j), isos[i].Vertex(j) + isos[i].VertexNormal(j) * l);
					s.DrawArrow(*scene, l / 2, pen, QBrush(c));
				}
			}
		}

		for (int c : children[i])
		{
			draw(c, level + 1);
		}
	};

	if (simple)
		for (int i = 0; i < Size(); ++i)
			draw(i, 0);
	else
		for (int r : roots)
			draw(r, 0);

	return scene;
}
