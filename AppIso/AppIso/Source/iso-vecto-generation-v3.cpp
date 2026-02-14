#include "iso-vecto-generation-v3.h"
#include "article-utils.h"

using namespace std;

#define INITIAL_VALUE Math::Infinity
#define NOT_INIT_ZONE_VALUE 99999999 // considering these values are not real height values

/*
 * Les graphes R, Z et P ont la meme topology (celle donnée par z)
 *
 * \param z Les différentes zones, qu'importe leur valeur, elles seront triées de la plus petite à la plus grande, on va toujours commencer par la plus petite
 * \param p Les valeurs de proba qu'on veut donner. Elles doivent se situer dans l'intervalle [0, 1]
 * \param exactT Permet de dire si on veut suivre exactement T ou non (si oui on peut avoir des zones endoréiques)
 */
IsoVectoGenerationV3::IsoVectoGenerationV3(const GraphPoisson& z, const ScalarField2& p, bool withEndoreicZones) : R(z, INITIAL_VALUE), Z(z), P(z), TLow(z), THigh(z), T(z), withEndoreicZones(withEndoreicZones)
{
	P.SetValueFromScalarField(p);
	for (int i = 0; i < P.Size(); ++i)
	{
		// Pour éviter les erreurs avec des probas nulles
		P[i] = Math::Min(0.999, Math::Max(0.001, P[i]));
	}
}

/*
 * Generate a graph poisson with value from 0 to n (the size of the graph)
 * The result can be recover with Result();
 *
 * \param debug Si != 0, nombre de debug à faire (un tous les `debug` frames) + les zones TLow, THigh et T
 * \param root Le dossier root + potentiel début de nom pour les images de debug
 */
GraphPoisson IsoVectoGenerationV3::Generate(int d, const QString& r)
{
	debug = d;
	root = r;

	if (debug > 0)
		ArticleUtils::ArticleGif(R, -1, root);

	PreProcess();

	if (debug > 0)
		ArticleUtils::ArticleGif(R, -1, root);

	while (nbAssigned != R.Size())
	{
		ProcessNextZone();
	}

	PostProcess();

	return R;
}

GraphPoisson IsoVectoGenerationV3::Result() const
{
	return R;
}

GraphPoisson IsoVectoGenerationV3::Proba() const
{
	return P;
}

GraphPoisson IsoVectoGenerationV3::Zones() const
{
	return Z;
}

QVector<GraphPoisson> IsoVectoGenerationV3::EdenAsc() const
{
	return edenAsc;
}

QVector <GraphPoisson> IsoVectoGenerationV3::EdenDesc() const
{
	return edenDesc;
}

QVector <GraphPoisson> IsoVectoGenerationV3::EdenFinal() const
{
	return edenFinal;
}

void IsoVectoGenerationV3::PreProcess()
{
	// RAZ
	nbAssigned = 0;
	lastChosen = -1;
	R = GraphPoisson(Z, INITIAL_VALUE);
	idZones.clear();
	edenAsc.clear();
	edenDesc.clear();
	edenFinal.clear();

	// Récupération de toutes les zones
	for (int nodeId = 0; nodeId < Z.Size(); ++nodeId)
	{
		idZones.insert(Z.At(nodeId));
	}
}

void IsoVectoGenerationV3::ProcessNextZone()
{
	if (idZones.isEmpty())
	{
		cerr << "[Version 3] There is no more zone to cover, the process should be over" << endl;
		exit(1);
	}

	currentZone = *std::min_element(idZones.begin(), idZones.end());
	idZones.remove(currentZone);

	CreateBorders();
	DoubleEdenNodesAssignments();
	FinalNodesAssignments();
}

void IsoVectoGenerationV3::PostProcess()
{

}

void IsoVectoGenerationV3::CreateBorders()
{
	internalBorders.clear();
	externalBorders.clear();
	currentZoneNodes.clear();

	// On assigne directement les éléments proche de la cote
	for (int nodeId = 0; nodeId < Z.Size(); ++nodeId)
	{
		if (Z.At(nodeId) == currentZone && Z.IsBorder(nodeId))
		{
			R[nodeId] = GetNextHeight();
		}
	}

	// Initialisation des Edens
	for (int nodeId = 0; nodeId < Z.Size(); ++nodeId)
	{
		// Un membre de la zone ne peut pas se situer sur une bordure dès le début
		// Il est possible que le noeud soit de la zone, mais soit déjà assigné (dans R donc) s'il était sur la bordure, donc on ne veut pas le réassigner.
		if (Z[nodeId] == currentZone && R[nodeId] == INITIAL_VALUE)
		{
			currentZoneNodes.insert(nodeId);
			TLow[nodeId] = NOT_INIT_ZONE_VALUE;
			THigh[nodeId] = NOT_INIT_ZONE_VALUE;
			T[nodeId] = NOT_INIT_ZONE_VALUE;
			R[nodeId] = NOT_INIT_ZONE_VALUE;
		}
		// Les bordures sont les éléments des zones externes qui sont au bord d'un élément de la zone actuelle
		else
		{
			TLow[nodeId] = INITIAL_VALUE;
			THigh[nodeId] = INITIAL_VALUE;
			T[nodeId] = INITIAL_VALUE;
			for (int neighId : Z.Neighbours(nodeId))
			{
				if (Z[neighId] == currentZone && (R[neighId] == INITIAL_VALUE || R[neighId] == NOT_INIT_ZONE_VALUE))
				{
					if (Z[nodeId] > currentZone)
						internalBorders.insert(nodeId);
					else
						externalBorders.insert(nodeId);
				}
			}
		}
	}

	// To keep all nodes in the zone directly neighbouring the external borders (possible growth from these elements)
	accessibleElements.clear();
	for (int nodeId : externalBorders)
	{
		for (int neighId : Z.Neighbours(nodeId))
		{
			if (R[neighId] == NOT_INIT_ZONE_VALUE)
			{
				accessibleElements.insert(neighId);
			}
		}
	}

	if (internalBorders.empty() && externalBorders.empty())
	{
		cerr << "[Version 3] No border in the Eden. Impossible to grow for zone " << currentZone << " without border." << endl;
	}
}

void IsoVectoGenerationV3::DoubleEdenNodesAssignments()
{
	int sizeZone = currentZoneNodes.size();
	for (int i = 0; i < sizeZone; ++i)
	{
		if (!DoubleEdenChooseNextNode(i, false))
		{
			//qDebug() << "[Version 3] Ascending eden stopped after " << i << "/" << sizeZone << "steps";
			break;
		}
	}
	for (int i = 0; i < sizeZone; ++i)
	{
		if (!DoubleEdenChooseNextNode(i, true))
		{
			//qDebug() << "[Version 3] Descending eden stopped after " << i << "/" << sizeZone << "steps";
			break;
		}
	}

	// T représente le pourcentage asc / (desc + asc) de l'eden
	// Plus on se rapproche de 1 et plus on est haut car proche de la barrière haute (et inversement)
	// Dans certain cas (sommet de montagne), on n'a pas de valeur desc, dans ce cas, on considère également une valeur dans [0, 1] pour pouvoir trier.
	// Inversement dans certain cas (zone endoréique), on n'a pas de valeur asc, on fait un calcul similaire pour avoir une valeur dans [0, 1].
	// Lorsqu'on est sur un sommet on donne la valeur (n étant le nombre d'éléments dans la zone)
	//			asc / n
	// Cela permet aux sommets proches d'être directement choisi et de ne pas empiéter sur ceux qui veulent aller haut
	// Avec le meme argument, pour les zones endoréique on va donner
	//			(n - desc) / n
	// Ainsi on se rapproche fort de n dans ces zones

	for (int nodeId : currentZoneNodes)
	{
		bool containsAsc = TLow[nodeId] != NOT_INIT_ZONE_VALUE;
		bool containsDesc = THigh[nodeId] != NOT_INIT_ZONE_VALUE;
		if (!containsAsc && !containsDesc)
		{
			cerr << "[Version 3] pas normal qu'un point ne soit couvert ni par un Eden, ni par l'autre." << endl;
		}
		else if (!containsDesc)
		{
			T[nodeId] = TLow[nodeId] / sizeZone;
		}
		else if (!containsAsc)
		{
			T[nodeId] = (sizeZone - THigh[nodeId]) / sizeZone;
		}
		else
		{
			T[nodeId] = TLow[nodeId] / (TLow[nodeId] + THigh[nodeId]);
		}
	}

	edenDesc.push_back(THigh);
	edenAsc.push_back(TLow);
	edenFinal.push_back(T);
}

bool IsoVectoGenerationV3::DoubleEdenChooseNextNode(int valueToGive, bool internal)
{
	QSet<int>& borders = internal ? internalBorders : externalBorders;
	GraphPoisson& Zone = internal ? THigh : TLow;

	static Random r;
	while (true)
	{
		// On récupère un élément de bordure (on inverse les proba si on vient de l'intérieur (phase descendante)
		int parent = GetRandomNode(borders, internal);

		// Certaines zones n'ont pas de bordures internes (isos de pics)
		// Certaines zones n'ont pas de bordures externes (isos endoréique)
		// Donc il est possible qu'on ne puisse plus grossir
		if (parent == -1)
		{
			return false;
		}

		// On choisit un des voisins de manière aléatoire
		QVector<int> neighs;
		for (int neigh : Z.Neighbours(parent))
		{
			if (Zone[neigh] == NOT_INIT_ZONE_VALUE)
			{
				neighs.append(neigh);
			}
		}

		// Un noeud est dans la bordure s'il a au moins un voisin non choisi
		// On enlève un noeud de l'ensemble des bordures que lorsqu'on le choisi aléatoirement et qu'on se rend compte que tous les voisins sont choisis
		// TODO: c'est sans doute plus rapide de garder le set le plus petit possible à chaque fois, mais c'est plus clair et plus rapide à coder comme ça
		if (!neighs.empty())
		{
			lastChosen = neighs[r.Integer(neighs.size())];
			borders.insert(lastChosen);
			Zone[lastChosen] = valueToGive;
			return true;
		}
		else
		{
			// On n'est plus dans la bordure si on n'a plus de voisin
			borders.remove(parent);
		}
	}
}

void IsoVectoGenerationV3::FinalNodesAssignments()
{
	int sizeZone = currentZoneNodes.size();

	// On assigne selon l'ordre de T, meme si cela fait des zones endoréiques
	if (withEndoreicZones)
	{
		QVector<QPair<int, double>> remainings;
		for (int nodeId : currentZoneNodes)
		{
			if (R[nodeId] == NOT_INIT_ZONE_VALUE)
			{
				remainings.append({ nodeId, T[nodeId] });
			}
		}
		std::sort(remainings.begin(), remainings.end(), [](const auto& a, const auto& b) { return a.second < b.second; });
		for (const auto& e : remainings)
		{
			R[e.first] = GetNextHeight();
		}
	}
	// On assigne selon l'ordre de T, mais seulement en fonction des éléments de T proche de la bordure actuelle
	else
	{
		for (int i = 0; i < sizeZone; ++i)
		{
			if (!FinalChooseNextNode())
			{
				qDebug() << "[Version 3] Empty borders while trying to choose a node. Verify input zones, you may have endorheic region! The"
					<< sizeZone - i << "remaining nodes are automatically put in the T order.";

				QVector<QPair<int, double>> remainings;
				for (int nodeId : currentZoneNodes)
				{
					if (R[nodeId] == NOT_INIT_ZONE_VALUE)
					{
						remainings.append({ nodeId, T[nodeId] });
					}
				}
				std::sort(remainings.begin(), remainings.end(), [](const auto& a, const auto& b) { return a.second < b.second; });
				for (const auto& e : remainings)
				{
					R[e.first] = GetNextHeight();
				}
				break;
			}
		}
	}
}

bool IsoVectoGenerationV3::FinalChooseNextNode()
{
	if (accessibleElements.empty())
	{
		return false;
	}

	// On récupère l'élément de la bordure à valeur la plus basse (PAS DE PROBA)
	lastChosen = *accessibleElements.begin();
	double smallest = T[lastChosen];
	for (int p : accessibleElements)
	{
		if (T[p] < smallest)
		{
			lastChosen = p;
			smallest = T[p];
		}
	}

	// Tous les voisins non assignés deviennent des éléments accessibles
	for (int neigh : Z.Neighbours(lastChosen))
	{
		if (R[neigh] == NOT_INIT_ZONE_VALUE)
		{
			accessibleElements.insert(neigh);
		}
	}

	accessibleElements.remove(lastChosen);
	R[lastChosen] = GetNextHeight();

	// Permet juste de faire une animation pour chaque choix
	if (debug > 0 && nbAssigned % debug == 0)
		ArticleUtils::ArticleGif(R, lastChosen, root);

	return true;
}

// Renvoie un noeud aléatoire, en fonction des bordures données. On inverse lorsqu'on part de la bordure externe pour que les probas signifient la meme chose
int IsoVectoGenerationV3::GetRandomNode(const QSet<int>& borders, bool inverseProba)
{
	static Random r = Random::R239;
	double cumul = 0;
	for (int ind : borders)
	{
		// TODO: normalement les probas vont de 0 à 1, donc c'est plus simple de faire ça pour inverser les probas, mais ptetre faire différemment
		if (inverseProba) cumul += 1 - P[ind];
		else			  cumul += P[ind];
	}

	// Si aucun point ne peut être choisi, on n'en choisi pas
	if (cumul == 0)
		return -1;

	double p = r.Uniform(0, cumul);
	for (int ind : borders)
	{
		if (inverseProba) p -= 1 - P[ind];
		else			  p -= P[ind];

		if (p <= 0)
		{
			//if (cumul < 1)
				//cout << cumul << " node " << ind << endl;
			return ind;
		}
	}

	std::cerr << "PROBLEM" << std::endl;
	return 0;
}

// Should only be called when we set the height of a node
double IsoVectoGenerationV3::GetNextHeight()
{
	return nbAssigned++;
}
