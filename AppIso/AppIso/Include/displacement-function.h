#pragma once
#include "iso-line.h"
#include "circle.h"
#include "evector.h"

#include <QtWidgets/QGraphicsScene>

/**
 * Classe permettant de d�crire une fonction de d�placement sur une ligne de [0, 1] -> [0, 1].
 * C'est une classe g�n�rique qui contient des fonctions d'inverse etc..
 *
 * La fonction doit �tre d�crite sur l'intervalle [0, 1] et doit avoir les propri�t�s suivantes :
 * - f(0) = f(1) = 0
 * - f est continue
 * - f(x) + x est croissante (<=> bijective dans ce contexte puisque f(0) + 0 = 0, f(1) + 1 = 1 et on est continue sur [0, 1], donc croissante <=> bijectif)
 *
 * Et dans l'id�al :
 * - f(x) >= 0			D�placement vers l'avant
 * - f(0.5) = max(f)	D�placement max au centre
 * - f'(0) = f'(1) = 0	D�placement C^2 dans le warping global
 */

class DisplacementFunction
{
protected:
	double factor = 1.; //!< Facteur de multiplication de la fonction, peut impacter certaines valeurs (notamment l'inverse).
	// /!\ WARNING : Si cette valeur est > FactorMax, la fonction n'est plus croissante et certains r�sultats sont non d�finis.
	// Attention � cette valeur de base � 1, il faut que f(x) + x soit donc d�j� croissant !

public:
	DisplacementFunction();

	double operator()(const double&) const;

	virtual double Value(double) const { return 0; }
	virtual double MinDerivative() const { return 0; }
	virtual double Max() const { return 0; }

	// Facteur max pour que `factor * f(x) + x` reste croissante.
	double FactorMax() const;
	double Factor() const;
	void ChangeFactor(double);

	// Par d�faut, on fait une dichotomie car `factor * f(x) + x` est croissante
	virtual double InverseWithId(double) const; // `(factor * f(x) + x)^{-1}`
	double ValueWithId(double) const;			// `factor * f(x) + x`
};

inline double DisplacementFunction::FactorMax() const
{
	double d = MinDerivative();
	return d == 0 ? Math::Infinity : -1 / MinDerivative();
}

inline double DisplacementFunction::operator()(const double& x) const
{
	return Value(x);
}

inline double DisplacementFunction::Factor() const
{
	return factor;
}

inline void DisplacementFunction::ChangeFactor(double f)
{
	if (f > FactorMax())
	{
		std::cerr << "Le facteur de multiplication de cette fonction doit �tre <= " << FactorMax() << "." << std::endl;
	}
	else
	{
		factor = f;
	}
}

inline double DisplacementFunction::ValueWithId(double x) const
{
	return factor * Value(x) + x;
}

// Fonction deplacement nulle
class DisplacementFunctionZero : public DisplacementFunction
{
public:
	DisplacementFunctionZero() {}

	double Value(double) const override { return 0; }
	double MinDerivative() const override { return 0; }
	double InverseWithId(double x) const override { return x; }
};

// Fonction (x(1-x))^n
// n >= 2 pour �tre C^2
class DisplacementFunction1 : public DisplacementFunction
{
protected:
	int k;

public:
	DisplacementFunction1(double ki) : k(Math::Max(1, ki)) {}

	double Value(double) const override;
	double MinDerivative() const override;

	double InverseWithId(double) const override;
};

inline double DisplacementFunction1::Value(double x) const
{
	return Math::Pow(x * (1 - x), k);
}

inline double DisplacementFunction1::MinDerivative() const
{
	if (k == 1)
		return -1.;

	return -(k / sqrt(2. * k - 1.)) * Math::Pow((k - 1.) / (4. * k - 2.), k - 1);
}

inline double DisplacementFunction1::InverseWithId(double x) const
{
	if (factor == 0)
		return x;

	// Cas sp�cifique avec k = 1
	if (k == 1)
	{
		double b = 1 + factor;
		return (b - sqrt(b * b - 4 * factor * x)) / (2 * factor);
	}

	return DisplacementFunction::InverseWithId(x);
}

/**
 * Classe permettant de d�crire une fonction f : R^2 -> R^2 de warping � support compact (un cercle) dans une direction dir.
 * Pour chaque point p, on regarde sa distance au cercle (de centre c et rayon r)
 * 
 *     f(p) = p              Si ||p - c|| > r : 
 *     f(p) = p + g(p - c)   Sinon
 * 
 * Avec g(p) une fonction d�fini comme
 * 
 *     g(p) = p + d(t)(b - a)
 * 
 * Avec b et a les intersection du point avec le cercle dans la direction de d�placement dir et t = ||p - a|| / ||b - a||.
 * 
 * On remarque que g(p) est �quivalent � dire
 * 
 *     g(p) = a + (d(t) + t)(b - a)
 * 
 * D'o� le fait que d(t) + t doit �tre bijectif (pour pouvoir revenir sur le point d'origine en connaissant le point d'arriv�e).
 */
class DisplacementCircle
{
protected:
	Vector2 c;		//!< Centre du cercle.
	double r;		//!< Rayon du cercle.
	Vector2 d;		//!< Direction de d�placement.
	DisplacementFunction* f; //!< Fonction de d�placement R -> R.

public:
	DisplacementCircle(const Vector2&, const double&, const Vector2&, DisplacementFunction*);

	Vector2 Displacement(const Vector2&, bool = false) const;
	Vector2 Center() const;
	Vector2 Direction() const;
	double Radius() const;
	Circle2 Support() const;

	void Draw(QGraphicsScene&, int = 10) const;
};

inline Vector2 DisplacementCircle::Center() const
{
	return c;
}

inline Vector2 DisplacementCircle::Direction() const
{
	return d;
}

inline double DisplacementCircle::Radius() const
{
	return r;
}

inline Circle2 DisplacementCircle::Support() const
{
	return Circle2(c, r);
}

/**
 * Classe permettant de warp un ensemble d'isos en fonction d'un ensemble de points (sample d'une courbe) donn� par l'utilisateur
 * 
 * Pour le moment, cette classe utilise obligatoirement la fonction de d�placement DisplacementFunction1
 */
class WarpingIsos
{
protected:
	IsoLines isosBefore;
	IsoLines isosAfter;
	QVector<Vector2> pathPoints; //!< Points de la trajectoire demand�e.
	QVector<Vector2> modifiedPathPoints; //!< Points de la trajectoire du dernier d�placement.

public:
	WarpingIsos();
	WarpingIsos(const IsoLines&);

	void PathDisplacement(double, double, double = 1, double = 2);

	void SetIsos(const IsoLines&);
	IsoLines WarpedIsos() const;
	void SetPathPoints(const QVector<Vector2>&);

protected:
	QVector<Vector2> AdaptPointsToParams(const QVector<Vector2>& , double) const;
	void WarpIsosAroundCircle(const DisplacementCircle&, IsoLines&, double) const;
	void WarpPointsAroundCircle(const DisplacementCircle&, QVector<Vector2>&) const;
};

inline void WarpingIsos::SetIsos(const IsoLines& nisos)
{
	isosBefore = nisos;
}

inline IsoLines WarpingIsos::WarpedIsos() const
{
	return isosAfter;
}

inline void WarpingIsos::SetPathPoints(const QVector<Vector2>& pts)
{
	pathPoints = pts;
}
