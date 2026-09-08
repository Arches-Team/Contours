#pragma once

#include "sampling.h"
#include "tin.h"
#include "histogramd.h"
#include "polygon.h"
#include "evector.h"

/*
 * Graphe � l'int�rieur d'un masque donn�, repr�sentant l� o� il y a du terrain et l� o� il y a de la "mer".
 * La g�n�ration est faite comme ceci :
 * - On �tend la box du masque d'une largeur de 4 * radius
 * - On poissonnise toute cette zone �tendu = topologyExt
 * - On ne garde que les points � l'int�rieur du masque = topology (l'endroit o� il y a du terrain)
 * 
 * Un point du graphe est consid�r� comme une bordure (ie proche de la mer) si
 * - Il est dans la zone du masque
 * - Il est adjacent � un sommet de topologyExt qui est dans la box du masque. En gros, si le bord du masque est � 1, on consid�re que ce n'est pas la mer mais l'int�rieur du continent. On ne veut donc pas que ce sommet soit un bord de mer, meme si c'est un bord de topology.
 * 
 * Le principe est de donner des valeurs de hauteurs aux points du terrain (ceux de topology) r�cup�rables et modifiables avec `At`
 * Puis on souhaite r�cup�rer les isolignes avec `ContourLines`
 * 
 * Pour r�cup�rer les isolignes on donne des valeurs � topologyExt et on fait un marching triangles
 * Pour �tre s�r de toujours avoir des polygones, on proc�de ainsi :
 * - Les valeurs de `topology` ne change pas
 * - les valeurs de la mer (`topologyExt` � l'int�rieur de la box du masque) sont mises un peu plus bas que le bord de mer
 * - les autres points de `topologyExt` voisins de `topology` (ceux hors de la box mais avec un voisin direct dans `topology`) sont mis � la valeur du voisin de `topology` le plus proche.
 *   Pourquoi ? Parce que ceci permet que les isolignes r�cup�r�s "sortent" de la box. On a ainsi toujours des polygones, et si on cut les isolignes dans la box du masque, on ne voit pas les contours moches du bords qui n'ont aucun sens
 * - les autres valeurs des noeuds de topologyExt sont mise � une valeur tr�s basse pour �tre sur que le marching triangle sorte toujours des polygones quelle que soit la hauteur.
 * 
 * Il n'y a pas d'int�ret � demander des contours � hauteur plus basse que la valeur de la mer, le r�sultat n'aurait aucun sens.
 * 
 * TLDR: tout est fait avec `topology`. `topologyExt` sert juste � d�finir les bordures exactes et � avoir des isolignes facilement r�cup�rable comme des polygones.
 * 
 * TODO: pour le moment, cette fa�on de faire ne g�re pas les contours qui sont tr�s fins. Par exemple, si l'utilisateur donne une crevasse fine repr�sentant une fine bande de mer qui rentre dans la terre, les triangles n'existent pas dans `topology` donc c'est parfait, mais l'extraction de l'isolignes "0" va consid�rer tous ces triangles comme � l'int�rieur de la zone car ils sont dans `topologyExt`. Si on ne les mets pas, l'iso ne serait pas r�cup�rable avec le marching triangles. On a deux solutions :
 * - consid�rer que si le sample n'est pas assez fin, on oublie juste la crevasse
 * - essayer de r�cup�rer les ar�tes qui rejoignent deux points qui traversent cette crevasse (en gros les voisins dans `topologyExt` qui ne sont pas dans `topology`), et dans ce cas, essayer de trouver une fa�on de donner l'iso externe � partir de l� (par exemple le niveau de la mer serait au milieu des deux points, donc si on veut prendre une iso plus haute, on verrait la crevasse), mais c'est pas �vident � impl�menter parce qu'on se retrouve avec potentiellement 1, 2 ou 4 points par triangle.
 */
class GraphPoisson
{
protected:
	double radius;
	ScalarField2 mask;
	QSharedPointer<Tin2> topology;	  // pointer to avoid duplication in copy constructor
	QSharedPointer<Tin2> topologyExt; // tous les points et triangles dans la box �tendue de 4*r
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
	
	// TODO: On utilise topologyExt car elle nous permet d'�viter les bugs de points seuls et ceux des points d'articulation qui ne sont pas pris en compte dans le TIN.
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
	// On est oblig� pour cela sinon ils embetent (ils peuvent �tre dans un coin de la carte donc pas en bordure, mais en meme temps personne ne peut y acc�der)
	// On le fait dans topologyExt car on utilise topologyExt pour connaitre les voisins, �a �vite au + les points seuls et les points d'articulation qui ne sont pas pris en compte dans le TIN.
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
			// Points dans la box d�fini par le masque = point de l'ext�rieur
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