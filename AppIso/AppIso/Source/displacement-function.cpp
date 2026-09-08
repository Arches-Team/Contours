#include "displacement-function.h"
#include "ray.h"

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
 * \brief Warp les isos le long de la courbe utilisateur repr�sent�e par les points pathPoints
 * 
 * \param radius	Le rayon du cercle repr�sentant le support compact des fonctions de d�placements
 * \param mu		[0, 1] La force = la vitesse de d�placement des points dans le cercle
 * \param n 		[1, inf[ Facteur de puissance de la fonction de d�placement
 * \param eps		]0, 1] Quantit� de samples 0 = erreur, 0+ = un max, 1 = minimum possible
 */
void WarpingIsos::PathDisplacement(double radius, double mu, double n, double eps)
{
	if (pathPoints.size() < 2 || eps <= 0)
		return;

	DisplacementFunction1 df(n);
	// Valeur max que l'on veut donner entre 2 points pour pouvoir amener le premier point sur le deuxi�me lorsque mu = 1
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
 * \brief Fait en sorte que les points de la courbe utilisateur aient une distance d'a peu pr�s epsMax entre chaque.
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

	// Nombre de segments � avoir
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
		// TODO: trouver le bon param�tre de resample
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
 * TODO: c'est un peu du c/c des autres m�thodes mais avec un ensemble de points au lieu d'un ensemble d'iso, il faudrait trouver une mani�re plus propre, mais j'ai pas le temps
 */