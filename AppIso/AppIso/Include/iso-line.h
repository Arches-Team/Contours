#pragma once

#include "heightfield.h"
#include "graph.h"
#include "histogramd.h"

class IsoLinePoly : public Polygon2
{
protected:
	double h; //<! Height of the iso
public:
	enum IntersectType { INTERSECT, INSIDE, CONTAINS, INDEPENDENT };
public:
	//! empty
	IsoLinePoly();
	IsoLinePoly(const QVector<Vector2>&, double = 0.0);
	IsoLinePoly(const Polygon2&, double = 0.0);

	double H() const;
	void SetH(double h);

	void Epurate(double eps);
	void Extend(double eps);
	double SignedArea() const;
	bool IsTrigo() const;
	bool IsAutoIntersecting() const;
	void ChangeOrder(bool = true);
	IsoLinePoly Resample(double) const;
	IsoLinePoly ResampleSpline(double, double = 0.5) const;

	Vector2 SmoothPoint(int, double, double) const;
	IsoLinePoly Smooth2(int, double = 0.1, int = 1) const; // old one with number of neighbours
	IsoLinePoly Smooth(double, double = 0.1, int = 1) const;
	IsoLinePoly SmoothInsideCircle(const Circle2&, double, double = 0.1, int = 1) const;
	IsoLinePoly LocalDisplacement(const Circle2&, double) const;
	IsoLinePoly DisplacementTowardCurve(const PointCurve2&, double, double, double = 2) const;

	IntersectType RelationWith(const Polygon2&) const;
	Vector2 VertexNormal(int) const;
	QVector<int> EarClip2() const;
};

/**
 * Isoline container which keep the fact that isolines are a set of non crossing polygons.
 * Check WellFormed() to see if the set of isolines is logical or not (meaning every siblings inside an iso at height h have all the same height or height h)
 */
class IsoLines
{
protected:
	QVector<IsoLinePoly> isos;
	QVector<int> parents;			//<! keep track of each isoline direct parent (i.e. the closest iso which contains it)
	QVector<QSet<int>> children;	//<! the inverse of `parents`
	QSet<int> roots;				//<! all isos which has no parent

	bool simple;		//<! If true, we only keep isolines without trying to know their parents and children, when this is true the Append function is really quicker

public:
	//! empty
	IsoLines(bool simple = false) : simple(simple) {};
	IsoLines(const QVector<Polygon2>&, double, double, bool = false);
	IsoLines(const Polygons2&, double, double, bool = false);
	IsoLines(const QVector<IsoLinePoly>&, bool = false);
	IsoLines(const ScalarField2&, const QSet<double>&, bool = false);
	IsoLines(const ScalarField2&, int, bool = false);
	IsoLines(const GraphPoisson&, const HistogramD&, bool = false);
	//IsoLines(const IsoLines& oth) : isos(oth.isos), parents(oth.parents), children(oth.children), roots(oth.roots) {}

	IsoLinePoly& operator[](int);
	IsoLinePoly operator[](int) const;

	// Access and modifications
	int Size() const;
	bool IsEmpty() const;
	IsoLinePoly At(int) const;
	Box2 GetBox() const;
	void GetRange(double&, double&) const;
	double AverageHeightBetweenIso() const;
	double HeightInside(int i) const;
	bool InternalBorder(int i) const;

	// Moves
	IsoLines Centered() const;

	int ComputeParentIso(const Vector2&) const;
	bool ComputeParentIso(const Polygon2&, int&) const;
	bool Append(const Polygon2&, double = 0);
	bool Append(const IsoLinePoly&);
	void Remove(int, bool = false);
	void Remove(const QSet<int>&, bool);
	void Remove(const QSet<int>&);

	// Relation between isolines
	int Parent(int) const;
	QSet<int> Children(int) const;
	QSet<int> Siblings(int) const;
	QSet<int> Roots() const;

	// Recursive information on levels
	int Levels() const;
	int Levels(int) const;
	bool WellFormed() const;
	bool WellFormed(int, bool = true) const;
	bool isEndoreic() const;
	bool isEndoreic(int) const;
	bool isGrowing(int) const;

	// Change heights of iso (without endoreic area)
	void ChangeHeightsMinMax(double, double);
	void ChangeHeightsMinMaxEndoreic(double, double);
	void ChangeHeights(double, double);
	void ChangeHeights(int, double, double);

	// Utils to change every isos without needed a loop
	void Smooth(double, double = 0.1, int = 1);
	void SmoothBig(int, double, double = 0.1, int = 1);
	void Resample(double);
	void ResampleSpline(double, double = 0.5);
	bool Inside(const Vector2&) const;

	// Information
	int VertexSize() const;
	double TotalLength() const;
	QSet<double> Heights() const;
	QVector<double> SortedHeights() const;
	HistogramD HeightsHistogram() const;

	// Debug
	ScalarField2 GetMask(int, int) const;
	ScalarField2 GetMask(const Box2&, int, int) const;

	struct DisplayOptions
	{
		bool fill = false;
		bool arrow = false;
		GenericPalette* palette = new AnalyticPalette(3);

		// multiplier for the edge size
		double edge_size = 1;

		// To only display the isos from level min_isos to max_isos
		// -1 = all isos
		int min_isos = -1;
		int max_isos = -1;
	};

	QGraphicsScene* ToScene(const DisplayOptions& = DisplayOptions()) const;

public:
	// Provide range-based for loops
	auto begin() { return isos.begin(); }
	auto end() { return isos.end(); }
	auto cbegin() const { return isos.begin(); }
	auto cend() const { return isos.end(); }
	auto begin() const { return isos.begin(); }
	auto end() const { return isos.end(); }
};

inline IsoLinePoly& IsoLines::operator[](int i)
{
	return isos[i];
}

inline IsoLinePoly IsoLines::operator[](int i) const
{
	return isos[i];
}

inline int IsoLines::Size() const
{
	return isos.size();
}

inline bool IsoLines::IsEmpty() const
{
	return isos.size() == 0;
}

inline IsoLinePoly IsoLines::At(int i) const
{
	return isos.at(i);
}

inline int IsoLines::Parent(int i) const
{
	return parents.at(i);
}

inline QSet<int> IsoLines::Children(int i) const
{
	return children.at(i);
}

inline QSet<int> IsoLines::Roots() const
{
	return roots;
}

inline QSet<int> IsoLines::Siblings(int i) const
{
	QSet<int> siblings;
	if (Parent(i) == -1)
		siblings = Roots();
	else
		siblings = Children(Parent(i));
	siblings.remove(i);
	return siblings;
}