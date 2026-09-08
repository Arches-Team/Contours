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

	// Getter
	IsoLines GetIsoLines() const { return isoLines; }

protected:
};