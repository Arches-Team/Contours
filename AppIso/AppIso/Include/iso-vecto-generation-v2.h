#pragma once
#include "histogramd.h"
#include "histogram.h"
#include "noise.h"
#include "graph.h"
#include "iso-line.h"

// Version 2 vectoriel
// Buts initiaux de la V2:
// - Copie de la version simple eden avec zones (V5) mais en vectoriel
// - garder les am�lioration de la V1 vectoriel

class IsoVectoGenerationV2
{
protected:
	GraphPoisson R;				// Les valeurs de hauteur
	GraphPoisson Z;				// Les diff�rentes zones
	GraphPoisson P;				// Les valeurs de proba (m�me graphe que Z)

	// parameters used in protected functions
	int nbAssigned = 0;
	int lastChosen = -1;

	double currentZone;
	QSet<int> currentZoneNodes;	// Ensemble des sommets du graphe dans la zone actuelle
	QSet<int> borders;			// Ensemble des sommets du graphe qui sont sur la bordure de la zone actuelle

	QSet<double> idZones;		// Les diff�rentes valeurs dans Z, repr�sentant les diff�rentes zones

	// debug
	int debug;
	QString root;

public:
	IsoVectoGenerationV2(const GraphPoisson&, const ScalarField2&);

	void Generate(int debug = 0, const QString& root = "");
	GraphPoisson Result() const;
	GraphPoisson Proba() const;
	GraphPoisson Zones() const;

protected:
	void PreProcess();
	void ProcessNextZone();
	void PostProcess();

	void CreateBorders();
	void NodesAssignments();
	bool ChooseNextNode();

	int GetRandomNode();
	double GetNextHeight();

public:
	static IsoVectoGenerationV2 TestGenerationHF(const ScalarField2&, const ScalarField2&, double);
};

inline IsoVectoGenerationV2 IsoVectoGenerationV2::TestGenerationHF(const ScalarField2& maskZones, const ScalarField2& noise, double r)
{
	GraphPoisson zones(maskZones, r);
	zones.SetStrictValueFromScalarField(maskZones);
	IsoVectoGenerationV2 gen(zones, noise);
	return gen;
}