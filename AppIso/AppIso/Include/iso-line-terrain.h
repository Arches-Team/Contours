#pragma once

#include "heightfield.h"
#include "cpu.h"
#include "realtime.h"
#include "iso-line.h"

class IsoLineTerrain
{
protected:

	IsoLines isoLines;				//!< All Iso
	double diffOutH;				//!< Height added outside all isos, usually, it is preferable if 0 <= diffOutH <= min iso heights
	double diffInH;					//!< Height added inside final isos, usually, it is preferable if 0 <= diffInH < min iso heights
	double outH;					//!< precomputed outside height to avoid redoing the computation each time (we can not precompute inH since it depends on the iso)

	QVector<double> isoHeights;		//!< Give height of isoline according to how deep they are in the tree

public:
	//! Empty (do not call)
	IsoLineTerrain() : diffOutH(0.0), diffInH(0.0) {};
	IsoLineTerrain(const IsoLines& isoLines, double outH = Math::Infinity, double inH = Math::Infinity);

	double InterpolateH(const Vector2& p, double distToFade = 0.0) const;
	HeightField InterpolateField(const Box2& b, int x, int y, double distToFade = 0.0) const;
	HeightField InterpolateField(int x, int y, double distToFade = 0.0) const;

	double StairsH(const Vector2& p) const;
	HeightField StairsField(const Box2& b, int x, int y) const;
	HeightField StairsField(int x, int y) const;

	HeightField SmoothStairsField(const Box2& b, int x, int y, int nsmooth = 5) const;
	HeightField SmoothStairsField(int x, int y, int nsmooth = 5) const;

	HeightField HeatField(const Box2& b, int x, int y, bool = true) const;
	HeightField HeatField(int x, int y, bool = true) const;

	QVector<Mesh> GetMesh(const Box2& box) const;
	QVector<Mesh> GetMesh() const;

	//! Tests
	static IsoLineTerrain TestSimpleTriangles();
	static IsoLineTerrain TestMoreTriangles();
	static IsoLineTerrain TestFromHeightField();
	static IsoLineTerrain TestVisualization(MeshWidget* widget);

	// Getter
	IsoLines GetIsoLines() const { return isoLines; }

protected:
};

inline IsoLineTerrain IsoLineTerrain::TestSimpleTriangles()
{
	Polygon2 t1(Vector2(2, 2), Vector2(6, 2), Vector2(4, 5));
	Polygon2 t2(Vector2(1, 1), Vector2(9, 1), Vector2(4, 6));
	Polygon2 t3(Vector2(8, 4), Vector2(8, 6), Vector2(10, 5));

	IsoLinePoly ilp1(t1, 1);
	IsoLinePoly ilp2(t2, 0);
	IsoLinePoly ilp3(t3, 0);

	IsoLines isos({ ilp1, ilp2, ilp3 });
	return IsoLineTerrain(isos);
}

inline IsoLineTerrain IsoLineTerrain::TestMoreTriangles()
{
	Polygon2 t1(Vector2(1, 2), Vector2(12.8, 2), Vector2(12.8, 10));
	Polygon2 t2(Vector2(13, 6), Vector2(13, 10), Vector2(16, 8));
	Polygon2 t3(Vector2(2, 2.5), Vector2(5, 2.5), Vector2(5, 4.5));
	Polygon2 t4(Vector2(13.2, 8), Vector2(13.2, 9), Vector2(14, 8.5));
	Polygon2 t5(Vector2(12, 8), Vector2(12, 4), Vector2(9, 6));
	
	//Polygon2 t6(Vector2(4, 0), Vector2(4, 3), Vector2(10, 0));

	IsoLinePoly ilp1(t1, 1);
	IsoLinePoly ilp2(t2, 1);
	IsoLinePoly ilp3(t3, 2);
	IsoLinePoly ilp4(t4, 2);
	IsoLinePoly ilp5(t5, 2);

	IsoLines isos({ ilp1, ilp2, ilp3, ilp3, ilp4 });
	return IsoLineTerrain(isos);
}

inline IsoLineTerrain IsoLineTerrain::TestFromHeightField()
{
	Box2 box(1000, 1000);
	QImage imghf = QImage("../Data/dems/rockies_zoom_breach.png");
	HeightField hf(box, imghf, 0, 300);

	return IsoLineTerrain(IsoLines(hf, 12));
}

inline IsoLineTerrain IsoLineTerrain::TestVisualization(MeshWidget* widget)
{
	Box2 box(1000, 1000);
	QImage imghf = QImage("../Data/dems/rockies_zoom_breach.png");
	HeightField hf(box, imghf, 0, 300);

	IsoLineTerrain ilt(IsoLines(hf, 12));
	ilt.InterpolateField(1000, 1000).CreateImage("test.png");
	ilt.StairsField(1000, 1000).CreateImage("test2.png");

	int i = 0;
	widget->ClearAll();
	for (Mesh& m : ilt.GetMesh())
	{
		widget->AddMesh("iso " + QString::number(i), m);
		++i;
	}

	return ilt;
}