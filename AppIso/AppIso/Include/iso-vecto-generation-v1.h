#pragma once
#include "histogramd.h"
#include "histogram.h"
#include "noise.h"
#include "graph.h"
#include "iso-line.h"
#include "draw.h"


// Version 1 vectoriel
// Buts initiaux de la V1:
// - Copie de la version V3 mais en vectoriel

class IsoVectoGenerationV1
{
protected:
	GraphPoisson R;				// Les valeurs de hauteur
	GraphPoisson P;				// Les valeurs de proba (même graphe que Z)

	int nbAssigned = 0;
	int lastChosen = -1;

	QSet<int> borders;			// Ensemble des sommets du graphe qui sont sur la bordure de la zone

public:
	IsoVectoGenerationV1(const GraphPoisson&, const ScalarField2&);

	void Generate(int debug = 0, const QString& root = "");
	GraphPoisson Result() const;
	GraphPoisson Proba() const;

protected:
	void PreProcess();
	void ChooseNextNode();
	void PostProcess();

	int GetRandomNode();
	double GetNextHeight();

public:
	static IsoVectoGenerationV1 TestGenerationHF(const ScalarField2&, const ScalarField2&, double);
};

inline IsoVectoGenerationV1 IsoVectoGenerationV1::TestGenerationHF(const ScalarField2& maskZones, const ScalarField2& noise, double r)
{
	GraphPoisson gp(maskZones, r);

	IsoVectoGenerationV1 gen(gp, noise);
	return gen;
}