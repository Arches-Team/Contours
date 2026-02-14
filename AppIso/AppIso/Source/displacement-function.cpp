#include "displacement-function.h"

using namespace std;

DisplacementFunction::DisplacementFunction() : factor(1)
{

}

double DisplacementFunction::InverseWithId(double x) const
{
	if (factor == 0)
		return x;

	double a = 0;
	double b = 1;
	double m;

	while (b - a > 1e-6)
	{
		m = (a + b) / 2.0;
		double v = ValueWithId(m);
		if (v == x)
			break;
		else if (v < x)
			a = m;
		else // (v > x)
			b = m;
	}

	return m;
}

DisplacementCircle::DisplacementCircle(const Vector2& center, const double& radius, const Vector2& dir, DisplacementFunction* func) : c(center), r(radius), d(Normalized(dir)), f(func)
{

}

Vector2 DisplacementCircle::Displacement(const Vector2& p, bool inverse) const
{
	double dist = r - Norm(p - c);

	if (dist <= 0)
		return p;
	
	Circle2 circle = Support();
	Ray2 ray = Ray2(p, d);
	double ta, tb;

	if (!circle.Intersect(ray, ta, tb))
		return p;

	Vector2 a = ray(ta);
	Vector2 b = ray(tb);
	
	double t = Norm(p - a) / Norm(b - a);
	double s;
	if (inverse)
		s = f->InverseWithId(t);
	else
		s = f->ValueWithId(t);

	return a + (b - a) * s;
}

ScalarField2 DisplacementCircle::CreateSF(int w, int h) const
{
	Box2 box(c, r * 1.2);
	ScalarField2 sf(box, w, h, 0);

	for (int x = 0; x < w; ++x)
	{
		for (int y = 0; y < h; ++y)
		{
			Vector2 p = sf.ArrayVertex(x, y);
			sf(x, y) = Math::Max(0, r - Norm(c - Displacement(p, true)));
			//sf(x, y) = Norm(Displacement(p) - p);
		}
	}
	// Pour pouvoir voir une vraie différence (sur une image) au cas où le centre n'est pas image de la fonction (pas normal)
	//sf(0, 0) = r;

	return sf;
}

void DisplacementCircle::Draw(QGraphicsScene& scene, int gridSize) const
{
	double maxi = 0;
	double scaling = 1.2;
	double total = 100;

	double width = r / (10 * gridSize);
	AnalyticPalette palette(3);

	// Columns
	QVector<Vector2> position;
	QVector<Vector2> warping;
	for (int i = 0; i <= gridSize ; ++i)
	{
		double x = c[0] + (2 * r * scaling) * (i / (double)gridSize) - r * scaling;
		for (int j = 0; j <= total; ++j)
		{
			double y = c[1] + (2 * r * scaling) * (j / (double)total) - r * scaling;
			Vector2 p(x, y);
			Vector2 w = Displacement(p);
			maxi = Math::Max(maxi, Norm(p - w));
			position.append(p);
			warping.append(w);
		}
	}

	for (int i = 0; i <= gridSize; ++i)
	{
		for (int j = 0; j < total; ++j)
		{
			int ind1 = i * (total + 1) + j;
			int ind2 = i * (total + 1) + j + 1;

			Vector2 pos1 = position[ind1];
			Vector2 warp1 = warping[ind1];
			QColor col1 = palette.GetColor(Norm(pos1 - warp1) / maxi).GetQt();

			Vector2 warp2 = warping[ind2];

			QPen pen(col1);
			pen.setWidth(width);
			scene.addLine(warp1[0], warp1[1], warp2[0], warp2[1], pen);
		}
	}

	// Lines
	position.clear();
	warping.clear();

	for (int i = 0; i <= gridSize; ++i)
	{
		double y = c[0] + (2 * r * scaling) * (i / (double)gridSize) - r * scaling;
		for (int j = 0; j <= total; ++j)
		{
			double x = c[1] + (2 * r * scaling) * (j / (double)total) - r * scaling;
			Vector2 p(x, y);
			Vector2 w = Displacement(p);
			maxi = Math::Max(maxi, Norm(p - w));
			position.append(p);
			warping.append(w);
		}
	}

	for (int i = 0; i <= gridSize; ++i)
	{
		for (int j = 0; j < total; ++j)
		{
			int ind1 = i * (total + 1) + j;
			int ind2 = i * (total + 1) + j + 1;

			Vector2 pos1 = position[ind1];
			Vector2 warp1 = warping[ind1];
			QColor col1 = palette.GetColor(Norm(pos1 - warp1) / maxi).GetQt();

			Vector2 warp2 = warping[ind2];

			QPen pen(col1);
			pen.setWidth(width);
			scene.addLine(warp1[0], warp1[1], warp2[0], warp2[1], pen);
		}
	}

	// Circle
	Circle2 circ(c, r);
	QPen pen;
	pen.setWidth(r / 100);
	pen.setColor(Qt::black);
	circ.Draw(scene, pen);
}

WarpingIsos::WarpingIsos() : isosBefore({}), isosAfter({})
{

}

WarpingIsos::WarpingIsos(const IsoLines& isos) : isosBefore(isos), isosAfter(isos)
{

}

/**
 * \brief Warp les isos le long de la courbe utilisateur représentée par les points pathPoints
 * 
 * \param radius	Le rayon du cercle représentant le support compact des fonctions de déplacements
 * \param mu		[0, 1] La force = la vitesse de déplacement des points dans le cercle
 * \param n 		[1, inf[ Facteur de puissance de la fonction de déplacement
 * \param eps		]0, 1] Quantité de samples 0 = erreur, 0+ = un max, 1 = minimum possible
 */
void WarpingIsos::PathDisplacement(double radius, double mu, double n, double eps)
{
	if (pathPoints.size() < 2 || eps <= 0)
		return;

	DisplacementFunction1 df(n);
	// Valeur max que l'on veut donner entre 2 points pour pouvoir amener le premier point sur le deuxième lorsque mu = 1
	double epsMax = (2. * radius * df(0.5)) * df.FactorMax() * eps;

	// TODO: Taux avec lequel on resample les polygones, il faut trouver la bonne valeur 
	double tau = epsMax / 10;

	modifiedPathPoints = AdaptPointsToParams(pathPoints, epsMax);
	isosAfter = isosBefore;

	for (int p = 0; p < modifiedPathPoints.size() - 1; ++p)
	{
		Vector2 vi = modifiedPathPoints[p];
		Vector2 vj = modifiedPathPoints[p + 1];
		Vector2 di = vj - vi;
		double dist = Norm(di); // = eps dans l'article
		Vector2 dir = Normalized(di);

		// Voir l'article
		df.ChangeFactor((mu * dist) / (2. * radius * df(0.5)));

		WarpIsosAroundCircle(DisplacementCircle(vi, radius, dir, &df), isosAfter, tau);
	}
}

/**
 * \brief Fait en sorte que les points de la courbe utilisateur aient une distance d'a peu près epsMax entre chaque.
 */
QVector<Vector2> WarpingIsos::AdaptPointsToParams(const QVector<Vector2>& path, double distMax) const
{
	double length = 0;
	Vector2 last = path[0];
	for (int i = 1; i < path.size(); ++i)
	{
		length += Norm(path[i] - last);
		last = path[i];
	}

	// Nombre de segments à avoir
	int size = Math::Ceil(length / distMax);
	double segsize = length / size;

	QVector<Vector2> points;
	QVector<Vector2> modifiedPath = path;
	points.reserve(size + 1);
	points.append(path.first());
	int ind = 0;
	double l = 0;
	for (int i = 0; i < size - 1; ++i)
	{
		Vector2 dir = modifiedPath[ind + 1] - modifiedPath[ind];
		double l2 = Norm(dir);
		if (l + l2 > segsize)
		{
			Vector2 p = modifiedPath[ind] + Normalized(dir) * (segsize - l);
			modifiedPath[ind] = p;
			points.append(p);
			l = 0;
		}
		else
		{
			l += l2;
			ind++;
			i--;

			if (ind >= modifiedPath.size() - 1)
			{
				cerr << "error" << endl;
				exit(1);
			}
		}
	}

	points.append(path.last());
	return points;
}

void WarpingIsos::WarpIsosAroundCircle(const DisplacementCircle& dc, IsoLines& isos, double distMax) const
{
	Circle2 support = dc.Support();
	for (IsoLinePoly& ilp : isos)
	{
		// TODO: trouver le bon paramètre de resample
		// TODO: resample uniquement dans le cercle permettrait de gagner beaucoup de temps
		ilp = ilp.Resample(distMax);
		for (int i = 0; i < ilp.Size(); ++i)
		{
			Vector2 bef = ilp.Vertex(i);
			Vector2 aft = dc.Displacement(bef);
			ilp.Vertex(i) = aft;
		}
	}
}

void WarpingIsos::WarpPointsAroundCircle(const DisplacementCircle& dc, QVector<Vector2>& pts) const
{
	Circle2 support = dc.Support();
	for (int i = 0; i < pts.size(); ++i)
	{
		Vector2 bef = pts[i];
		Vector2 aft = dc.Displacement(bef);
		pts[i] = aft;
	}
}

/**
 * \brief Same as above (PathDisplacement) except that isolines are not modified, the modification is done on a grid and draw on a scene
 * TODO: c'est un peu du c/c des autres méthodes mais avec un ensemble de points au lieu d'un ensemble d'iso, il faudrait trouver une manière plus propre, mais j'ai pas le temps
 */
void WarpingIsos::DrawGrid(QGraphicsScene& scene, double radius, double mu, int gridSize, double eps, int n)
{
	if (pathPoints.size() < 2 || eps <= 0)
		return;

	DisplacementFunction1 df(n);
	// Valeur max que l'on veut donner entre 2 points pour pouvoir amener le premier point sur le deuxième lorsque mu = 1
	double epsMax = (2. * radius * df(0.5)) * df.FactorMax() * eps;

	// TODO: Taux avec lequel on resample les polygones, il faut trouver la bonne valeur 
	double tau = epsMax / 10;

	QVector<Vector2> modifiedPathPointsDraw = AdaptPointsToParams(pathPoints, epsMax);

	double scaling = 1.2;
	double total = 1000;

	Box2 b = Box2::Null;
	for (Vector2 c : modifiedPathPointsDraw)
	{
		b = Box2(b, Box2(c, radius * scaling));
	}
	int sizeX = gridSize;
	int sizeY = gridSize;
	if (b.Width() > b.Height())
	{
		sizeY = Math::Ceil((b.Height() / b.Width()) * gridSize);
	}
	else
	{
		sizeX = Math::Ceil((b.Width() / b.Height()) * gridSize);
	}
	double stepX = b.Width() / sizeX;
	double stepY = b.Height() / sizeY;
	double stepTotX = b.Width() / total;
	double stepTotY = b.Height() / total;

	// Columns
	QVector<Vector2> position;
	QVector<Vector2> warping;
	for (int i = 0; i <= sizeX; ++i)
	{
		double x = b[0][0] + i * stepX;
		for (int j = 0; j <= total; ++j)
		{
			double y = b[0][1] + j * stepTotY;
			position.append(Vector2(x, y));
			warping.append(Vector2(x, y));
		}
	}

	for (int p = 0; p < modifiedPathPointsDraw.size() - 1; ++p)
	{
		Vector2 vi = modifiedPathPointsDraw[p];
		Vector2 vj = modifiedPathPointsDraw[p + 1];
		Vector2 di = vj - vi;
		double dist = Norm(di); // = eps dans l'article
		Vector2 dir = Normalized(di);

		// Voir l'article
		df.ChangeFactor((mu * dist) / (2. * radius * df(0.5)));

		WarpPointsAroundCircle(DisplacementCircle(vi, radius, dir, &df), warping);

		// Circle
		Circle2 circ(vi, radius);
		QPen pen;
		//pen.setWidth(Norm(b.Diagonal()) / 400);
		//pen.setColor(Qt::black);
		//circ.Draw(scene, pen);
	}

	double maxi = 0;
	for (int i = 0; i < position.size(); ++i)
	{
		maxi = Math::Max(maxi, Norm(position[i] - warping[i]));
	}

	double width = Norm(b.Diagonal()) / (20 * gridSize);

	for (int i = 0; i < pathPoints.size() - 1; ++i)
	{
		QPen pen(Qt::darkGreen);
		pen.setWidth(width);
		scene.addLine(pathPoints[i][0], pathPoints[i][1], pathPoints[i + 1][0], pathPoints[i + 1][1], pen);
	}

	AnalyticPalette palette(3);

	for (int i = 0; i <= sizeX; ++i)
	{
		for (int j = 0; j < total; ++j)
		{
			int ind1 = i * (total + 1) + j;
			int ind2 = i * (total + 1) + j + 1;

			Vector2 pos1 = position[ind1];
			Vector2 warp1 = warping[ind1];
			QColor col1 = palette.GetColor(Norm(pos1 - warp1) / maxi).GetQt();

			Vector2 warp2 = warping[ind2];
			Vector2 pos2= position[ind2];

			QPen pen(col1);
			pen.setWidth(width);
			scene.addLine(warp1[0], warp1[1], warp2[0], warp2[1], pen);
			//scene.addLine(pos1[0], pos1[1], pos2[0], pos2[1], pen);
		}
	}

	// Lines
	//position.clear();
	//warping.clear();
	//for (int i = 0; i <= sizeY; ++i)
	//{
	//	double y = b[0][1] + i * stepY;
	//	for (int j = 0; j <= total; ++j)
	//	{
	//		double x = b[0][0] + j * stepTotX;
	//		position.append(Vector2(x, y));
	//		warping.append(Vector2(x, y));
	//	}
	//}

	//for (int p = 0; p < modifiedPathPointsDraw.size() - 1; ++p)
	//{
	//	Vector2 vi = modifiedPathPointsDraw[p];
	//	Vector2 vj = modifiedPathPointsDraw[p + 1];
	//	Vector2 di = vj - vi;
	//	double dist = Norm(di); // = eps dans l'article
	//	Vector2 dir = Normalized(di);

	//	// Voir l'article
	//	df.ChangeFactor((mu * dist) / (2. * radius * df(0.5)));

	//	WarpPointsAroundCircle(DisplacementCircle(vi, radius, dir, &df), warping);
	//}

	//maxi = 0;
	//for (int i = 0; i < position.size(); ++i)
	//{
	//	maxi = Math::Max(maxi, Norm(position[i] - warping[i]));
	//}

	//for (int i = 0; i <= sizeY; ++i)
	//{
	//	for (int j = 0; j < total; ++j)
	//	{
	//		int ind1 = i * (total + 1) + j;
	//		int ind2 = i * (total + 1) + j + 1;

	//		Vector2 pos1 = position[ind1];
	//		Vector2 warp1 = warping[ind1];
	//		QColor col1 = palette.GetColor(Norm(pos1 - warp1) / maxi).GetQt();

	//		Vector2 warp2 = warping[ind2];
	//		Vector2 pos2 = position[ind2];

	//		QPen pen(col1);
	//		pen.setWidth(width);
	//		scene.addLine(warp1[0], warp1[1], warp2[0], warp2[1], pen);
	//		//scene.addLine(pos1[0], pos1[1], pos2[0], pos2[1], pen);
	//	}
	//}
}