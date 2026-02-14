#pragma once

#include "sampling.h"
#include "tin.h"
#include "histogramd.h"

/*
 * Graphe à l'intérieur d'un masque donné, représentant là où il y a du terrain et là où il y a de la "mer".
 * La génération est faite comme ceci :
 * - On étend la box du masque d'une largeur de 4 * radius
 * - On poissonnise toute cette zone étendu = topologyExt
 * - On ne garde que les points à l'intérieur du masque = topology (l'endroit où il y a du terrain)
 * 
 * Un point du graphe est considéré comme une bordure (ie proche de la mer) si
 * - Il est dans la zone du masque
 * - Il est adjacent à un sommet de topologyExt qui est dans la box du masque. En gros, si le bord du masque est à 1, on considère que ce n'est pas la mer mais l'intérieur du continent. On ne veut donc pas que ce sommet soit un bord de mer, meme si c'est un bord de topology.
 * 
 * Le principe est de donner des valeurs de hauteurs aux points du terrain (ceux de topology) récupérables et modifiables avec `At`
 * Puis on souhaite récupérer les isolignes avec `ContourLines`
 * 
 * Pour récupérer les isolignes on donne des valeurs à topologyExt et on fait un marching triangles
 * Pour être sûr de toujours avoir des polygones, on procède ainsi :
 * - Les valeurs de `topology` ne change pas
 * - les valeurs de la mer (`topologyExt` à l'intérieur de la box du masque) sont mises un peu plus bas que le bord de mer
 * - les autres points de `topologyExt` voisins de `topology` (ceux hors de la box mais avec un voisin direct dans `topology`) sont mis à la valeur du voisin de `topology` le plus proche.
 *   Pourquoi ? Parce que ceci permet que les isolignes récupérés "sortent" de la box. On a ainsi toujours des polygones, et si on cut les isolignes dans la box du masque, on ne voit pas les contours moches du bords qui n'ont aucun sens
 * - les autres valeurs des noeuds de topologyExt sont mise à une valeur très basse pour être sur que le marching triangle sorte toujours des polygones quelle que soit la hauteur.
 * 
 * Il n'y a pas d'intéret à demander des contours à hauteur plus basse que la valeur de la mer, le résultat n'aurait aucun sens.
 * 
 * TLDR: tout est fait avec `topology`. `topologyExt` sert juste à définir les bordures exactes et à avoir des isolignes facilement récupérable comme des polygones.
 * 
 * TODO: pour le moment, cette façon de faire ne gère pas les contours qui sont très fins. Par exemple, si l'utilisateur donne une crevasse fine représentant une fine bande de mer qui rentre dans la terre, les triangles n'existent pas dans `topology` donc c'est parfait, mais l'extraction de l'isolignes "0" va considérer tous ces triangles comme à l'intérieur de la zone car ils sont dans `topologyExt`. Si on ne les mets pas, l'iso ne serait pas récupérable avec le marching triangles. On a deux solutions :
 * - considérer que si le sample n'est pas assez fin, on oublie juste la crevasse
 * - essayer de récupérer les arêtes qui rejoignent deux points qui traversent cette crevasse (en gros les voisins dans `topologyExt` qui ne sont pas dans `topology`), et dans ce cas, essayer de trouver une façon de donner l'iso externe à partir de là (par exemple le niveau de la mer serait au milieu des deux points, donc si on veut prendre une iso plus haute, on verrait la crevasse), mais c'est pas évident à implémenter parce qu'on se retrouve avec potentiellement 1, 2 ou 4 points par triangle.
 */
class GraphPoisson
{
protected:
	double radius;
	ScalarField2 mask;
	QSharedPointer<Tin2> topology;	  // pointer to avoid duplication in copy constructor
	QSharedPointer<Tin2> topologyExt; // tous les points et triangles dans la box étendue de 4*r
	QVector<int> topoToExt;			  // comment passer d'un indice de `topology` vers `topologyExt`
	QVector<int> extToTopo;			  // l'inverse (= -1 lorsque le point n'est pas dans `topology`)
	QVector<double> values;

	friend class ArticleUtils;

public:
	GraphPoisson() : radius(0), topology(nullptr), topologyExt(nullptr), values({}) {};
	GraphPoisson(const ScalarField2&, double, const double& = 0);
	//GraphPoisson(const ScalarField2&, double, const ScalarField2&);
	//GraphPoisson(const GraphPoisson&);
	GraphPoisson(const GraphPoisson&, const double&);

	double operator[](int) const;
	double& operator[](int);
	double At(int) const;
	double& At(int);

	void SetValueFromScalarField(const ScalarField2&);
	void SetStrictValueFromScalarField(const ScalarField2&);
	void SetValueFromHistogram(const HistogramD&);
	
	void GetRange(double&, double&) const;
	QSet<double> Values() const;
	QVector<double> SortedValues() const;
	Box2 GetBox() const;
	QSharedPointer<Tin2> Topology() const;
	double Radius() const;
	int Size() const;

	QSet<int> Neighbours(int) const;
	bool IsBorder(int) const;
	Vector2 Position(int) const;
	
	double Value(const Vector2&) const;
	ScalarField2 Rasterize(const Box2&, int, int) const;
	ScalarField2 Rasterize(int, int) const;

	double SeaLevel() const;
	Polygons2 ContourLines(double h) const;

	struct DisplayOptions
	{
		// Colors
		GenericPalette* palette = new AnalyticPalette(3);
		QColor edgeColor = Qt::black;
		double edgeRatio = 0.1;
		QColor outlineColor = Qt::black;
		double outlineRatio = 0.1;

		QColor uncoloredNodeColor = Qt::white;
		double uncoloredOutlineRatio = 0.1;

		QColor externalNodeColor = Color(255, 255, 255, 50).GetQt();
		double externalOutlineRatio = 0.1;

		QColor borderNodeColor = Color(240, 150, 150).GetQt();
		double borderOutlineRatio = 0.1;

		QColor selectedNodeColor = Color(255, 192, 0).GetQt();
		double selectedOutlineRatio = 0.4;

		double inRangeBorderOutlineRatio = 0.4;

		// Different nodes to display
		bool displayBorder = false;
		bool displayExternalNodes = false;
		
		// Color node within a range of values
		double minColoredVal = -Math::Infinity;
		double maxColoredVal = Math::Infinity;
		bool displayMaxValNodes = true;  // specific outline for the biggest nodes (the selected node)
		bool displayMaxSpecificColor = true;  // specific color for the biggest nodes (the selected node)

		// Display node within a range of values
		double minDisplayedVal = -Math::Infinity;
		double maxDisplayedVal = Math::Infinity;

		// Values to be used within the palette (if infinity, use the min/max of nodes)
		double minVal = -Math::Infinity;
		double maxVal = Math::Infinity;
	};

	QGraphicsScene* ToScene(const DisplayOptions& = DisplayOptions()) const;

protected:
	bool ExteriorPointInsideMask(int vi) const;
};

inline double GraphPoisson::operator[](int i) const
{
	return values[i];
}

inline double& GraphPoisson::operator[](int i)
{
	return values[i];
}

inline double GraphPoisson::At(int i) const
{
	return values[i];
}

inline double& GraphPoisson::At(int i)
{
	return values[i];
}

inline QSharedPointer<Tin2> GraphPoisson::Topology() const
{
	return topology;
}

// The radius used to generate the Poisson
inline double GraphPoisson::Radius() const
{
	return radius;
}

inline int GraphPoisson::Size() const
{
	return values.size();
}

inline QSet<int> GraphPoisson::Neighbours(int i) const
{
	QSet<int> neighs;
	
	// TODO: On utilise topologyExt car elle nous permet d'éviter les bugs de points seuls et ceux des points d'articulation qui ne sont pas pris en compte dans le TIN.
	int ei = topoToExt[i];
	for (int ni : topologyExt->VertexNeighboursVertices(ei))
	{
		int eni = extToTopo[ni];
		if (eni != -1)
			neighs.insert(eni);
	}

	return neighs;
}

inline bool GraphPoisson::IsBorder(int i) const
{
	if (!topology->IsBorderVertex(i))
		return false;

	int ei = topoToExt[i];
	// On est obligé pour cela sinon ils embetent (ils peuvent être dans un coin de la carte donc pas en bordure, mais en meme temps personne ne peut y accéder)
	// On le fait dans topologyExt car on utilise topologyExt pour connaitre les voisins, ça évite au + les points seuls et les points d'articulation qui ne sont pas pris en compte dans le TIN.
	//if (topology->IsAloneVertex(i))
	//	return true;
	if (topologyExt->IsAloneVertex(ei))
		return true;
	for (int eni : topologyExt->VertexNeighboursVertices(ei))
	{
		int ni = extToTopo[eni];
		// Voisin de la topologie globale qui n'est pas dans le masque = potentielle bordure
		if (ni == -1)
		{
			// Points dans la box défini par le masque = point de l'extérieur
			if (ExteriorPointInsideMask(eni))
			{
				return true;
			}
		}
	}
	return false;
}

inline Vector2 GraphPoisson::Position(int i) const
{
	return topology->Vertex(i);
}