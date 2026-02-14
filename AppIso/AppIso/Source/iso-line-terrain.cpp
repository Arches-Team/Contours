#include "iso-line-terrain.h"
#include "iso-line.h"
#include "draw.h"

using namespace std;

IsoLineTerrain::IsoLineTerrain(const IsoLines& isoLines, double diffOutH, double diffInH) : isoLines(isoLines), diffOutH(diffOutH), diffInH(diffInH)
{
	double a, b;
	isoLines.GetRange(a, b);
	double diff = isoLines.AverageHeightBetweenIso();
	if (Math::Abs(diff) < 0.0001)
		diff = 1;

	if (diffOutH == Math::Infinity)
	{
		this->diffOutH = diff;
	}
	if (diffInH == Math::Infinity)
	{
		this->diffInH = diff / 2;
	}
	this->outH = a - this->diffOutH;
}

/*!
\brief Interpolate between heights of iso and child isos according to point position
\warn Do not render endoreic areas well, [ENDOREIC] comments are for problems arising with endoreic isolines.

\param distToFade Distance to pass from first iso height to outH height (to avoid instant cliff)
*/
double IsoLineTerrain::InterpolateH(const Vector2& p, double distToFade) const
{
	double epsilon = 0.0001;
	int i1 = isoLines.ComputeParentIso(p);

	// Cas où on est à l'extérieur des isos
	if (i1 == -1)
	{
		double dist = Math::Infinity;
		double h1 = isoLines[*isoLines.Roots().begin()].H();
		for (int c : isoLines.Roots())
		{
			dist = Math::Min(dist, fabs(isoLines[c].Signed(p)));
		}

		double h2 = h1 - diffOutH;

		if (dist > distToFade)
			return h2;

		// Pour éviter les imprécisions numériques
		if (dist < epsilon)
			return h1;

		// Interpolation linéaire entre h1 et h2
		return (h2 * dist + h1 * (distToFade - dist)) / distToFade;
	}

	// Cas à l'intérieur d'une iso
	double h1 = isoLines[i1].H();

	// Distance par rapport à l'iso extérieure
	double d1 = fabs(isoLines[i1].Signed(p));

	// Plus petite distance par rapport à toutes les isos enfants
	double d2 = Math::Infinity;
	int i2 = 0;
	for (int c : isoLines.Children(i1))
	{
		double d = fabs(isoLines[c].Signed(p));
		// [ENDOREIC] Small correction for endoreic areas, permettant de récupérer la vraie zone la plus proche
		if (isoLines[c].H() == h1)
		{
			if (d1 > d)
			{
				d1 = d;
				i1 = c; // on l'utilise jamais mais au cas où
			}
		}
		else
		{
			if (d2 > d)
			{
				d2 = d;
				i2 = c;
			}
		}
	}

	// Si d2 vaut l'infini c'est qu'on a deux possibilité
	// 1. On a pas d'enfant (on est un pic - ou un creux si endoreic)
	// 2. [ENDOREIC] On a des enfants mais ils sont tous à la meme hauteur -> on est aussi un bassin ou un pic (juste il y a des zones endoreics)
	// On utilise la distance au centre de l'iso comme point le plus haut et on interpole
	if (d2 == Math::Infinity)
	{
		Vector2 center = isoLines[i1].Center();
		d2 = Norm(center - p);

		double h2 = h1 + diffInH;
		// Small correction for endoreic areas
		if (!isoLines.isGrowing(i1))
			h2 = Math::Max(outH, h1 - diffInH);

		// Pour éviter les imprécisions numériques
		if (d2 < epsilon)
			return h2;

		// Interpolation linéaire entre outH et h
		return (d1 * h2 + d2 * h1) / (d2 + d1);
	}

	// Hauteur intérieur
	double h2 = isoLines[i2].H();

	// Pour éviter les imprécisions numériques
	if (d2 < epsilon)
	{
		return h2;
	}

	// Interpolation des hauteurs en fonction des distances
	return (d1 * h2 + d2 * h1) / (d1 + d2);
}

/*!
\brief Returns the height field of this terrain given a certain box and precision - with interpolate style

\param b The box.
\param x Precision in x.
\param y Precision in y.
\param distToFade Distance to pass from first iso height to outH height (to avoid instant cliff)
*/
HeightField IsoLineTerrain::InterpolateField(const Box2& b, int x, int y, double distToFade) const
{
	HeightField sf(b, x, y);

	//#pragma omp parallel for
	for (int i = 0; i < x; ++i)
	{
		for (int j = 0; j < y; ++j)
		{
			sf(i, j) = InterpolateH(sf.Vertex(i, j), distToFade);
		}
	}

	return sf;
}

/*!
\brief Returns the height field of this terrain in the box containing all iso lines - with interpolate style

\param x Precision in x.
\param y Precision in y.
\param distToFade Distance to pass from first iso height to outH height (to avoid instant cliff)
*/
HeightField IsoLineTerrain::InterpolateField(int x, int y, double distToFade) const
{
	return InterpolateField(isoLines.GetBox(), x, y, distToFade);
}

/*!
\brief Heights as directly given by the set of isos
*/
double IsoLineTerrain::StairsH(const Vector2& p) const
{
	int parent = isoLines.ComputeParentIso(p);
	if (parent == -1)
		return outH;
	return isoLines.HeightInside(parent);
}

/*!
\brief Returns the height field of this terrain given a certain box and precision - with stairs style

\param b The box.
\param x Precision in x.
\param y Precision in y.
*/
HeightField IsoLineTerrain::StairsField(const Box2& b, int x, int y) const
{
	HeightField sf(b, x, y);

	#pragma omp parallel for
	for (int i = 0; i < x; ++i)
	{
		for (int j = 0; j < y; ++j)
		{
			sf(i, j) = StairsH(sf.Vertex(i, j));
		}
	}

	return sf;
}

/*!
\brief Returns the height field of this terrain in the box containing all iso lines - with stairs style

\param x Precision in x.
\param y Precision in y.
*/
HeightField IsoLineTerrain::StairsField(int x, int y) const
{
	return StairsField(isoLines.GetBox(), x, y);
}

/*!
\brief Returns the height field of this terrain given a certain box and precision - with stairs style, but smoothed to allow a better rendering

\param b The box.
\param x Precision in x.
\param y Precision in y.
*/
HeightField IsoLineTerrain::SmoothStairsField(const Box2& b, int x, int y, int nsmooth) const
{
	IsoLines smoothedIsos = isoLines;
	for (IsoLinePoly& ilp : smoothedIsos)
	{
		ilp = ilp.ResampleSpline(Norm(b.Diagonal()) / (0.5 * (x + y)));
	}
	IsoLineTerrain ilt(smoothedIsos);
	HeightField hf = ilt.StairsField(b, x, y);
	hf.Smooth(nsmooth);
	return hf;
}

/*!
\brief Returns the height field of this terrain in the box containing all iso lines - with stairs style, but smoothed to allow a better rendering

\param x Precision in x.
\param y Precision in y.
*/
HeightField IsoLineTerrain::SmoothStairsField(int x, int y, int nsmooth) const
{
	return SmoothStairsField(isoLines.GetBox(), x, y, nsmooth);
}

/*!
\brief Returns the height field of this terrain given a certain box - with heat equation done on isos

\param x Precision in x.
\param y Precision in y.
\param baseInter if the diffusion is based on interpolation field or on the stairs field
*/
#include "diffusion.h"
HeightField IsoLineTerrain::HeatField(const Box2& b, int x, int y, bool baseInter) const
{
	HeightField stairs = StairsField(b, x, y);
	HeightField base = baseInter ? InterpolateField(b, x, y) : stairs;
	HeightField mask(b, x, y, 0);
	for (int i = 0; i < x; ++i)
	{
		for (int j = 0; j < y; ++j)
		{
			if (i == 0 || j == 0 || i == x - 1 || j == y - 1)
			{
				if (stairs(i, j) != diffOutH)
				{
					mask(i, j) = 1;
				}
			}
			else
			{
				double v = stairs(i, j);
				//if (stairs(i + 1, j) < v)
				//	mask(i, j) = 1;
				//if (stairs(i - 1, j) < v)
				//	mask(i, j) = 1;
				//if (stairs(i, j + 1) < v)
				//	mask(i, j) = 1;
				//if (stairs(i, j - 1) < v)
				//	mask(i, j) = 1;
				for (int ii = i - 1; ii <= i + 1; ++ii)
				{
					for (int jj = j - 1; jj <= j + 1; ++jj)
					{
						if (stairs(ii, jj) < v)
						{
							mask(i, j) = 1;
						}
					}
				}
			}
		}
	}

	Diffusion diff = Diffusion(base, mask);
	diff.BuildGL();
	diff.CompleteVcycle();
	return diff.GetResult();
}

/*!
\brief Returns the height field of this terrain in the box containing all iso lines - with heat equation done on isos

\param x Precision in x.
\param y Precision in y.
*/
HeightField IsoLineTerrain::HeatField(int x, int y, bool baseInter) const
{
	return HeatField(isoLines.GetBox(), x, y, baseInter);
}

/*!
\brief Returns the extruded meshes of all isolines in the given box

\param b The box
*/
QVector<Mesh> IsoLineTerrain::GetMesh(const Box2& b) const
{
	QVector<Mesh> meshes;

	// TODO: find goor baseHeight
	double baseHeight = Norm(b.Diagonal()) / 100;

	// TODO: faire en sorte de ne draw que dans la box
	for (IsoLinePoly ilp : isoLines)
	{
		double h = ilp.H();
		//ilp = ilp.ResampleSpline(100);
		//ilp.Epurate(10);
		ilp.ChangeOrder(true);
		QVector<int> triangles = ilp.EarClip2();
		Mesh mesh = Mesh::Extrude(Mesh2(ilp.Vertices(), triangles),  h - baseHeight, h);
		meshes.push_back(mesh);
	}

	meshes.push_back(Mesh::Extrude(Mesh2(b, 10, 10), diffOutH - baseHeight, diffOutH));

	return meshes;
}

/*!
\brief Returns the extruded meshes of all isolines
*/
QVector<Mesh> IsoLineTerrain::GetMesh() const
{
	return GetMesh(isoLines.GetBox());
}