#include "iso-vecto-generation-v2.h"
#include "article-utils.h"

using namespace std;

#define INITIAL_VALUE Math::Infinity
#define NOT_INIT_ZONE_VALUE 99999999 // considering these values are not real height values

/*
 * Les graphes R, Z et P ont la meme topology (celle donnée par z)
 *
 * \param z Les différentes zones, qu'importe leur valeur, elles seront triées de la plus petite à la plus grande, on va toujours commencer par la plus petite
 * \param p Les valeurs de proba qu'on veut donner. Elles doivent se situer dans l'intervalle [0, 1]
 */
IsoVectoGenerationV2::IsoVectoGenerationV2(const GraphPoisson& z, const ScalarField2& p) : R(z, INITIAL_VALUE), Z(z), P(z)
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
 * \param debug Si != 0, nombre de debug à faire (un tous les `debug` frames)
 * \param root Le dossier root + potentiel début de nom pour les images de debug
 */
void IsoVectoGenerationV2::Generate(int d, const QString& r)
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

	if (debug > 0)
		ArticleUtils::ArticleGif(R, -1, root);

	PostProcess();
}

GraphPoisson IsoVectoGenerationV2::Result() const
{
	return R;
}

GraphPoisson IsoVectoGenerationV2::Proba() const
{
	return P;
}

GraphPoisson IsoVectoGenerationV2::Zones() const
{
	return Z;
}

void IsoVectoGenerationV2::PreProcess()
{
	// RAZ
	nbAssigned = 0;
	lastChosen = -1;
	R = GraphPoisson(Z, INITIAL_VALUE);
	idZones.clear();

	// Récupération de toutes les zones
	for (int nodeId = 0; nodeId < Z.Size(); ++nodeId)
	{
		idZones.insert(Z.At(nodeId));
	}
}

void IsoVectoGenerationV2::ProcessNextZone()
{
	if (idZones.isEmpty())
	{
		cerr << "[Version 2] There is no more zone to cover, the process should be over" << endl;
		exit(1);
	}

	currentZone = *std::min_element(idZones.begin(), idZones.end());
	idZones.remove(currentZone);

	CreateBorders();
	NodesAssignments();
}

void IsoVectoGenerationV2::PostProcess()
{

}

void IsoVectoGenerationV2::CreateBorders()
{
	borders.clear();
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
			R[nodeId] = NOT_INIT_ZONE_VALUE;
		}
		// La bordure consiste en les éléments de la zone plus basse qui sont au bord d'un élément de la zone actuelle
		// Lorsque c'est la première zone, la bordure est constitué des gens déjà défini (d'où le <= et pas <)
		else if (Z[nodeId] <= currentZone)
		{
			for (int neighId : Z.Neighbours(nodeId))
			{
				if (Z[neighId] == currentZone && (R[neighId] == INITIAL_VALUE || R[neighId] == NOT_INIT_ZONE_VALUE))
				{
					borders.insert(nodeId);
				}
			}
		}
	}

	if (borders.empty())
	{
		cerr << "[Version 2] No border in the Eden. Impossible to grow for zone " << currentZone << " without border." << endl;
	}
}

void IsoVectoGenerationV2::NodesAssignments()
{
	int sizeZone = currentZoneNodes.size();
	for (int i = 0; i < sizeZone; ++i)
	{
		if (!ChooseNextNode())
		{
			qDebug() << "[Version 2] Empty borders while trying to choose a node. Verify input zones, you may have endorheic region! The"
				<< sizeZone - i << "remaining nodes are automatically put in the next zone.";

			double idNextZone = *std::min_element(idZones.begin(), idZones.end());
			for (int nodeId : currentZoneNodes)
			{
				if (R[nodeId] == NOT_INIT_ZONE_VALUE)
				{
					Z[nodeId] = idNextZone;
					R[nodeId] = INITIAL_VALUE;
				}
			}
			break;
		}
	}
}

#include "cpu.h"
bool IsoVectoGenerationV2::ChooseNextNode()
{
	static Random r;
	while (true)
	{
		// On récupère un élément de bordure
		int parent = GetRandomNode();

		if (parent == -1)
		{
			return false;
		}

		// On choisit un des voisins de manière aléatoire
		QVector<int> neighs;
		for (int neigh : Z.Neighbours(parent))
		{
			if (R[neigh] == NOT_INIT_ZONE_VALUE)
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
			R[lastChosen] = GetNextHeight();

			// Permet juste de faire une animation pour chaque choix
			if (debug > 0 && nbAssigned % debug == 0)
				ArticleUtils::ArticleGif(R, lastChosen, root);

			return true;
		}
		else
		{
			// On n'est plus dans la bordure si on n'a plus de voisin
			borders.remove(parent);
		}
	}
}


// Renvoie un noeud aléatoire, en fonction des bordures données.
int IsoVectoGenerationV2::GetRandomNode()
{
	static Random r = Random::R239;
	double cumul = 0;
	for (int ind : borders)
	{
		cumul += P[ind];
	}

	// Si aucun point ne peut être choisi, on n'en choisi pas
	if (cumul == 0)
		return -1;

	double p = r.Uniform(0, cumul);
	for (int ind : borders)
	{
		p -= P[ind];
		
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
double IsoVectoGenerationV2::GetNextHeight()
{
	return nbAssigned++;
}