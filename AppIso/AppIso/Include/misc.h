#pragma once
#include "sampling.h"

class Misc
{
public:
	static Mesh2 DelaunayPointsInBox(const Box2& box, double r);
	static QVector<int> IndicesBetweenMeshes(const Mesh2& m1, const Mesh2& m2, const double& mindist);
};