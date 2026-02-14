#include "iso-vecto-generation-v1.h"
#include "article-utils.h"

using namespace std;

#define INITIAL_VALUE Math::Infinity

/*
 * Les graphes R, Z et P ont la meme topology (celle donnée par z)
 *
 * \param z Le graphe sur lequel se baser pour la génération
 * \param p Les valeurs de proba qu'on veut donner. Elles doivent se situer dans l'intervalle [0, 1]
 */
IsoVectoGenerationV1::IsoVectoGenerationV1(const GraphPoisson& z, const ScalarField2& p) : R(z, INITIAL_VALUE), P(z)
{
	P.SetValueFromScalarField(p);
	for (int i = 0; i < P.Size(); ++i)
	{
		// Pour éviter les erreurs avec des probas nulles
		P[i] = Math::Max(0.001, P[i]);
	}
}

/*
 * Generate a graph poisson with value from 0 to n (the size of the graph)
 * The result can be recover with Result();
 * 
 * \param debug Si != 0, nombre de debug à faire (un tous les `debug` frames)
 * \param root Le dossier root + potentiel début de nom pour les images de debug
 */
void IsoVectoGenerationV1::Generate(int debug, const QString& root)
{
	if (debug > 0)
		ArticleUtils::ArticleGif(R, -1, root);

	PreProcess();

	if (debug > 0)
		ArticleUtils::ArticleGif(R, -1, root);

	while (nbAssigned != R.Size())
	{
		ChooseNextNode();
		if (debug > 0 && nbAssigned % debug == 0)
			ArticleUtils::ArticleGif(R, lastChosen, root);
	}

	if (debug > 0)
		ArticleUtils::ArticleGif(R, -1, root);

	PostProcess();
}

GraphPoisson IsoVectoGenerationV1::Result() const
{
	return R;
}

GraphPoisson IsoVectoGenerationV1::Proba() const
{
	return P;
}

void IsoVectoGenerationV1::PreProcess()
{
	// RAZ
	nbAssigned = 0;
	lastChosen = -1;

	for (int nodeId = 0; nodeId < R.Size(); ++nodeId)
	{
		// Toutes les bordures font directement partie de la première iso
		if (R.IsBorder(nodeId))
		{
			borders.insert(nodeId);
			R[nodeId] = GetNextHeight();
		}
	}

	if (borders.isEmpty())
	{
		cerr << "[Version 1] There is no border in the zone, this is not possible to grow. Undefined results." << endl;
	}
}

#include "cpu.h"
void IsoVectoGenerationV1::ChooseNextNode()
{
	static Random r;
	while (true)
	{
		// On récupère un élément de bordure
		int parent = GetRandomNode();

		if (parent == -1)
		{
			cerr << "[Version 1] Impossible to keep growing, no border remains" << endl;
			//System::SaveSvg(R.ToScene(), "Figures/test.svg");
			exit(1);
		}

		// On choisit un des voisins de manière aléatoire
		QVector<int> neighs;
		for (int neigh : R.Neighbours(parent))
		{
			if (R[neigh] == INITIAL_VALUE)
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
			R[lastChosen] = GetNextHeight();
			borders.insert(lastChosen);
			return;
		}
		else
		{
			// On n'est plus dans la bordure si on n'a plus de voisin
			borders.remove(parent);
		}
	}
}

void IsoVectoGenerationV1::PostProcess()
{

}

int IsoVectoGenerationV1::GetRandomNode()
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
double IsoVectoGenerationV1::GetNextHeight()
{
	return nbAssigned++;
}
