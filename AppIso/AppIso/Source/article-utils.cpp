#include <QtWidgets/QGraphicsTextItem>

#include "cpu.h"
#include "article-utils.h"
#include "misc.h"
#include "iso-line.h"

Random ArticleUtils::random;

const QColor ColorOutside = Color(160, 160, 160).GetQt();
const QColor ColorBorderNode = Color(97, 130, 234).GetQt();
const QColor ColorExtBorderNode = Color(220, 94, 75).GetQt();
const QColor ColorNodeOutside = Color(255, 255, 255, 0).GetQt();
const QColor ColorNode = Color(255, 255, 255).GetQt();
const QColor ColorRoot = Color(255, 192, 0).GetQt();

QGraphicsScene* ArticleUtils::GifForArticle(const GraphPoisson& Z, double noVal, double min, double max, int lastChosen, bool displayBorder, const AnalyticPalette& palette)
{
	QGraphicsScene* scene = new QGraphicsScene;

	if (Z.Size() == 0)
		return scene;

	double r = Z.Radius();
	Box2 b = Z.GetBox();
	Vector2 border = 2 * Vector2(r, r);

	//Box2(b[0] - border, b[1] + border).Draw(*scene, QPen(Qt::white), QBrush(Qt::white));

	QSet<int> borders;

	for (int i = 0; i < Z.Size(); ++i)
	{
		Vector2 p1 = Z.Position(i);
		for (int n : Z.Neighbours(i))
		{
			Vector2 p2 = Z.Position(n);
			QPen pen(QBrush(Qt::black), r / 10);
			if (Z.At(i) == noVal || Z.At(n) == noVal)
			{
				QColor c = ColorOutside;
				pen.setColor(c);
			}
			scene->addLine(p1[0], p1[1], p2[0], p2[1], pen);
			if (Z.At(i) != noVal && Z.At(n) == noVal && !borders.contains(i))
				borders.insert(i);
		}
	}

	for (int i = 0; i < Z.Size(); ++i)
	{
		Vector2 p = Z.Position(i);
		double k = max == min ? 0 : (Z.At(i) - min) / (max - min);
		QColor col = palette.GetColor(k).GetQt();
		QPen pen(QBrush(Qt::black), r / 10);
		if (displayBorder && borders.contains(i))
			pen = QPen(QBrush(Qt::black), r / 5);
		if (i == lastChosen)
			col = Qt::red;
		if (Z.At(i) == noVal)
		{
			col = ColorNode;
			QColor c = ColorOutside;
			pen.setColor(c);
		}
		scene->addEllipse(p[0] - r, p[1] - r, 2 * r, 2 * r, pen, QBrush(col));
	}

	return scene;
}

#define INITIAL_VALUE Math::Infinity

void ArticleUtils::ArticleGif(const GraphPoisson& Z, int lastChosen, const QString& root)
{
	static int j = 0;
	QGraphicsScene* scene = GifForArticle(Z, INITIAL_VALUE, 0, Z.Size() - 1, lastChosen);
	System::FlipVertical(*scene);
	System::SaveSvg(scene, root + "gen_" + QString::number(j) + ".svg");

	//static double maxheight = H.GetValue(H.MaxValue());
	//double h = H.GetValue(H.MaxValue());
	//scene = new QGraphicsScene;
	//scene->addRect(0, 0, 100, 50, QPen(QColor(0, 0, 0, 0)));
	//H.Draw(*scene, AnalyticPalette(3), 100, 50 * h / maxheight);
	//System::FlipVertical(*scene);
	//System::SaveSvg(scene, root + "histo_" + QString::number(j) + ".svg");

	j++;
}

void ArticleUtils::GraphConstructionDebug(const ScalarField2& mask, double r, const QString& root)
{
	GraphPoisson gp(mask, r, 1);
	qDebug() << gp.Size();
	QGraphicsScene* scene = gp.ToScene();
	//IsoLinePoly iso(mask.LineSegments(0.5).GetPolygons().At(0));
	IsoLinePoly iso(gp.ContourLines(0.5).At(0));
	//IsoLinePoly mink_iso = iso;
	//mink_iso.Expand(4 * r);
	iso = iso.ResampleSpline(0.1);
	//mink_iso = mink_iso.ResampleSpline(1);

	scene->addPolygon(iso.GetQt(), QPen(Qt::darkRed, 3));
	//scene->addPolygon(mink_iso.GetQt(), QPen(Qt::darkRed, 3));

	for (int i = 0; i < gp.topologyExt->VertexSize(); ++i)
	{
		if (gp.extToTopo[i] == -1)
		{
			Vector2 p = gp.topologyExt->Vertex(i);
			if (iso.Signed(p) < 4 * gp.Radius())
			{
				scene->addEllipse(p[0] - r, p[1] - r, 2 * r, 2 * r, QPen(ColorOutside, r / 10), QBrush(ColorNodeOutside));

				for (int ni : gp.topologyExt->VertexNeighboursVertices(i))
				{
					Vector2 p2 = gp.topologyExt->Vertex(ni);
					if (iso.Signed(p2) < 4 * gp.Radius())
					{
						Vector2 d = Normalized(p2 - p);
						Vector2 p1 = p + d * r;
						p2 = p2 - d * r;
						auto* item = scene->addLine(p1[0], p1[1], p2[0], p2[1], QPen(ColorOutside, r / 10));
					}
				}
			}
		}
	}
	System::FlipVertical(*scene);
	System::SaveSvg(scene, root + "mikowski.svg");
}

void ArticleUtils::EdenArticle(int sample, int n, const QString& name)
{
	double radius = 10;
	EdenGrowth2 eden(Vector2(0.0, 0.0), radius, n);
	eden.Step(sample);

	QGraphicsScene* scene = new QGraphicsScene;
	AnalyticPalette palette(3);

	double r = eden.Radius() / sqrt(2);
	QVector<int> topo = eden.GetIndices();
	double minX = eden.At(0).Center()[0];
	double maxX = eden.At(0).Center()[0];
	double minY = eden.At(0).Center()[1];
	double maxY = eden.At(0).Center()[1];
	for (int i = 0; i < eden.Size(); ++i)
	{
		Vector2 c = eden.At(i).Center();
		QColor col = palette.GetColor(i / (double)eden.Size()).GetQt();
		if (i == 0)
			col = ColorRoot;

		scene->addEllipse(c[0] - r, c[1] - r, 2 * r, 2 * r, QPen(QBrush(Qt::black), r / 10), QBrush(col));
		if (topo[i] != -1)
		{
			Vector2 c2 = eden.At(topo[i]).Center();
			auto item = scene->addLine(c2[0], c2[1], c[0], c[1], QPen(QBrush(col), r / 2));
			item->setZValue(-1000);
		}

		minX = Math::Min(minX, c[0]);
		maxX = Math::Max(maxX, c[0]);
		minY = Math::Min(minY, c[1]);
		maxY = Math::Max(maxY, c[1]);
	}

	const double eps = 0.075;
	const double rad = radius * (1 + eps);
	if (n == 4)
	{
		int mx = 0;
		int my = 0;
		int nx = (maxX - minX + 0.01) / (2 * rad);
		int ny = (maxY - minY + 0.01) / (2 * rad);
		for (int x = mx; x <= nx; x++)
		{
			double sy = minY + 2 * rad * my;
			double ey = minY + 2 * rad * ny;
			double px = minX + 2 * rad * x;
			Vector2 start(px, sy);
			Vector2 end(px, ey);
			auto item = scene->addLine(start[0], start[1], end[0], end[1], QPen(QBrush(ColorOutside), r / 8));
			item->setZValue(-2000);
		}
		for (int y = my; y <= ny; y++)
		{
			double sx = minX + 2 * rad * mx;
			double ex = minX + 2 * rad * nx;
			double py = minY + 2 * rad * y;
			Vector2 start(sx, py);
			Vector2 end(ex, py);
			auto item = scene->addLine(start[0], start[1], end[0], end[1], QPen(QBrush(ColorOutside), r / 8));
			item->setZValue(-2000);
		}
	}

	if (n == 6)
	{
		int nx = (maxX - minX + 0.01) / (2 * rad) + 1;
		int ny = (maxY - minY + 0.01) / (sqrt(3) * rad);
		for (int x = -ny / 2; x <= nx; x++)
		{
			int sx = Math::Max(0, 2 * x);
			int sy = Math::Max(0, -2 * x);
			int ex = Math::Min(2 * nx, 2 * x + ny);
			int ey = Math::Min(ny, 2 * (nx - x));
			Vector2 start(minX + sx * rad, minY + sy * rad * sqrt(3));
			Vector2 end(minX + ex * rad, minY + ey * rad * sqrt(3));
			auto item = scene->addLine(start[0], start[1], end[0], end[1], QPen(QBrush(ColorOutside), r / 8));
			item->setZValue(-2000);
		}
		for (int x = 0; x <= nx + ny / 2; x++)
		{
			int sx = Math::Min(2 * nx, 2 * x);
			int sy = Math::Max(0, 2 * (x - nx));
			int ex = Math::Max(0, 2 * x - ny);
			int ey = Math::Min(ny, 2 * x);
			Vector2 start(minX + sx * rad, minY + sy * rad * sqrt(3));
			Vector2 end(minX + ex * rad, minY + ey * rad * sqrt(3));
			auto item = scene->addLine(start[0], start[1], end[0], end[1], QPen(QBrush(ColorOutside), r / 8));
			item->setZValue(-2000);
		}
		for (int y = 0; y <= ny; y++)
		{
			double py = minY + rad * sqrt(3) * y;
			Vector2 start(minX, py);
			Vector2 end(minX + nx * rad * 2, py);
			auto item = scene->addLine(start[0], start[1], end[0], end[1], QPen(QBrush(ColorOutside), r / 8));
			item->setZValue(-2000);
		}
	}

	System::SaveSvg(scene, name);
}

void ArticleUtils::EdenOnTin(int sample, const QString& name)
{
	double radius = 10;
	ScalarField2 mask(Box2(radius * 0.2 * sample), 100, 100, 1);
	GraphPoisson gp(mask, radius, -1);

	// Closest point to vector2(0, 0)
	int startingPoint = 0;
	for (int i = 0; i < gp.Size(); ++i)
		if (Norm(gp.Position(i)) < Norm(gp.Position(startingPoint)))
			startingPoint = i;

	int s = 1;
	QVector<int> boundary = { startingPoint };
	QVector<int> parents(gp.Size(), -1);
	gp.At(startingPoint) = s;

	while (s < sample && !boundary.empty())
	{
		int c = random.Integer(boundary.size());
		int select = boundary[c];
		QVector<int> neighbours;
		for (int n : gp.Neighbours(select))
			if (gp.At(n) == -1)
				neighbours.append(n);

		if (neighbours.empty())
		{
			boundary[c] = boundary[boundary.size() - 1];
			boundary.removeLast();
		}
		else
		{
			int c2 = random.Integer(neighbours.size());
			int newcell = neighbours[c2];
			gp.At(newcell) = ++s;
			parents[newcell] = select;
			boundary.append(newcell);
		}
	}

	QGraphicsScene* scene = new QGraphicsScene;
	AnalyticPalette palette(3);

	double r = radius / sqrt(2) * 1.05;
	double minX = gp.Position(startingPoint)[0];
	double maxX = gp.Position(startingPoint)[0];
	double minY = gp.Position(startingPoint)[1];
	double maxY = gp.Position(startingPoint)[1];

	for (int i = 0; i < gp.Size(); ++i)
	{
		bool inside = gp.At(i) != -1;
		Vector2 c = gp.Position(i);
		if (inside)
		{
			QColor col = palette.GetColor(gp.At(i) / sample).GetQt();
			if (gp.At(i) == 1)
				col = ColorRoot;
			scene->addEllipse(c[0] - r, c[1] - r, 2 * r, 2 * r, QPen(QBrush(Qt::black), r / 10), QBrush(col));
			if (parents[i] != -1)
			{
				Vector2 c2 = gp.Position(parents[i]);
				auto item = scene->addLine(c2[0], c2[1], c[0], c[1], QPen(QBrush(col), r / 2));
				item->setZValue(-1000);
			}

			minX = Math::Min(minX, c[0]);
			maxX = Math::Max(maxX, c[0]);
			minY = Math::Min(minY, c[1]);
			maxY = Math::Max(maxY, c[1]);
		}
	}

	Box2 box(Vector2(minX, minY) - 4 * Vector2(radius, radius), Vector2(maxX, maxY) + 4 * Vector2(radius, radius));

	for (int i = 0; i < gp.Size(); ++i)
	{
		Vector2 c = gp.Position(i);
		if (box.Inside(c))
		{
			for (int n : gp.Neighbours(i))
			{
				Vector2 c2 = gp.Position(n);
				if (box.Inside(c2))
				{
					auto item = scene->addLine(c2[0], c2[1], c[0], c[1], QPen(QBrush(ColorOutside), r / 8));
					item->setZValue(-2000);
				}
			}
		}
	}

	System::SaveSvg(scene, name);
}

void ArticleUtils::DoubleEdenBoundaries(const GraphPoisson& gp, const QString& root)
{
	QSet<double> val;
	for (int i = 0; i < gp.Size(); ++i)
		val.insert(gp.At(i));

	QVector<double> values(val.begin(), val.end());
	std::sort(values.begin(), values.end());

	int nbZones = values.size();
	QVector<QGraphicsScene*> scenes(nbZones);
	for (int i = 0; i < nbZones; ++i)
		scenes[i] = new QGraphicsScene;

	QVector<int> indexScene(gp.Size());
	for (int i = 0; i < gp.Size(); ++i)
		for (int j = 0; j < nbZones; ++j)
			if (values[j] == gp.At(i))
				indexScene[i] = j;

	double r = gp.Radius();
	double min, max;
	gp.GetRange(min, max);
	Box2 b = gp.GetBox();

	for (int i = 0; i < gp.Size(); ++i)
	{
		Vector2 p = gp.Position(i);
		for (int n : gp.Neighbours(i))
		{
			Vector2 p2 = gp.Position(n);
			Vector2 d = Normalized(p2 - p);
			Vector2 p1 = p + d * r;
			p2 = p2 - d * r;
			for (int s = 0; s < nbZones; ++s)
			{
				bool in = indexScene[i] == s || indexScene[n] == s;
				if (!in)
				{
					bool ini = false;
					for (int ni : gp.Neighbours(i))
					{
						ini |= indexScene[ni] == s;
					}
					bool inn = false;
					for (int nn : gp.Neighbours(n))
					{
						inn |= indexScene[nn] == s;
					}
					in = ini && inn;
				}

				QColor col = in ? Qt::black : ColorOutside;
				scenes[s]->addLine(p1[0], p1[1], p2[0], p2[1], QPen(col, r / 10));
			}
		}
	}

	for (int i = 0; i < gp.Size(); ++i)
	{
		Vector2 p = gp.Position(i);
		for (int s = 0; s < nbZones; ++s)
		{
			QColor colborder = ColorOutside;
			QColor colnode = ColorNodeOutside;

			if (indexScene[i] == s)
			{
				colborder = Qt::black;
				colnode = ColorNode;
				
				if (s == 0 && gp.IsBorder(i))
				{
					colnode = ColorBorderNode;
				}
			}
			else if (indexScene[i] == s + 1)
			{
				for (int n : gp.Neighbours(i))
				{
					if (indexScene[n] == s)
					{
						colborder = Qt::black;
						colnode = ColorExtBorderNode;
						break;
					}
				}
			}
			else if (i != 0 && indexScene[i] == s - 1)
			{
				for (int n : gp.Neighbours(i))
				{
					if (indexScene[n] == s)
					{
						colborder = Qt::black;
						colnode = ColorBorderNode;
						break;
					}
				}
			}
			scenes[s]->addEllipse(p[0] - r, p[1] - r, 2 * r, 2 * r, QPen(colborder, r / 10), QBrush(colnode));
		}
	}

	for (int s = 0; s < nbZones; ++s)
	{
		System::FlipVertical(*scenes[s]);
		System::SaveSvg(scenes[s], root + "double_eden_boundaries_" + QString::number(s + 1) + ".svg");
	}
}