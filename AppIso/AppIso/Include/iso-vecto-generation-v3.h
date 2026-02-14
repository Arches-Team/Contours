#pragma once
#include "histogramd.h"
#include "histogram.h"
#include "noise.h"
#include "graph.h"
#include "iso-line.h"
#include "draw.h"


// Version 3 vectoriel
// Buts initiaux de la V3:
// - Copie de la version double eden (V6) mais en vectoriel
// - garder les amélioration de la V1 vectoriel

class IsoVectoGenerationV3
{
protected:
	GraphPoisson R;				// Les valeurs de hauteur
	GraphPoisson Z;				// Les différentes zones
	GraphPoisson P;				// Les valeurs de proba (même graphe que Z)

	// parameters used in protected functions
	GraphPoisson TLow;			// Valeurs temporaires pour le eden montant
	GraphPoisson THigh;			// Valeurs temporaires pour le eden descendant
	GraphPoisson T;				// Valeurs finales pour le choix des noeuds

	int nbAssigned = 0;
	int lastChosen = -1;

	double currentZone;
	bool withEndoreicZones;
	QSet<int> currentZoneNodes;			// Ensemble des sommets du graphe dans la zone actuelle
	QSet<int> internalBorders;			// Ensemble des sommets du graphe qui sont sur la bordure de la zone interne (haute)
	QSet<int> externalBorders;			// Ensemble des sommets du graphe qui sont sur la bordure de la zone externe (basse)
	QSet<int> accessibleElements;		// Ensemble des sommets voisins de la bordure actuel (permettant de savoir lequel est le plus petit a assigner lorsque withEndoreicZones = false)

	QSet<double> idZones; // Les différentes valeurs dans Z, représentant les différentes zones
	QVector<GraphPoisson> edenAsc; // To store the double eden growth if needed
	QVector<GraphPoisson> edenDesc;
	QVector<GraphPoisson> edenFinal;

	// debug
	int debug = 0;
	QString root = "";

public:
	IsoVectoGenerationV3(const GraphPoisson&, const ScalarField2&, bool = false);

	GraphPoisson Generate(int debug = 0, const QString& root = "");
	GraphPoisson Result() const;
	GraphPoisson Proba() const;
	GraphPoisson Zones() const;
	QVector<GraphPoisson> EdenAsc() const;
	QVector<GraphPoisson> EdenDesc() const;
	QVector<GraphPoisson> EdenFinal() const;

protected:
	void PreProcess();
	void ProcessNextZone();
	void PostProcess();

	void CreateBorders();
	void DoubleEdenNodesAssignments();
	bool DoubleEdenChooseNextNode(int, bool);
	void FinalNodesAssignments();
	bool FinalChooseNextNode();

	int GetRandomNode(const QSet<int>&, bool);
	double GetNextHeight();

public:
	static IsoVectoGenerationV3 TestGenerationHF(const ScalarField2&, const ScalarField2&, double, bool = false);
};

inline IsoVectoGenerationV3 IsoVectoGenerationV3::TestGenerationHF(const ScalarField2& maskZones, const ScalarField2& noise, double r, bool withEndoreicZones)
{
	GraphPoisson zones(maskZones, r);
	zones.SetStrictValueFromScalarField(maskZones);

	IsoVectoGenerationV3 gen(zones, noise, withEndoreicZones);
	return gen;
}