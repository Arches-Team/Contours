#pragma once
#include "graph.h"
#include "histogramd.h"
#include "eden.h"

class ArticleUtils
{
public:
	/*
	 * Affichage de l'avancée de la génération
	 */
	static QGraphicsScene* GifForArticle(const GraphPoisson& Z, double noVal, double min, double max, int lastChosen, bool displayBorder = true, const AnalyticPalette& palette = AnalyticPalette(3));
	static void ArticleGif(const GraphPoisson& Z, int lastChosen, const QString& root);
	static void GraphConstructionDebug(const ScalarField2& mask, double r, const QString& root);
	static void EdenArticle(int sample, int n, const QString& name);
	static void EdenOnTin(int sample, const QString& name);
	static void DoubleEdenBoundaries(const GraphPoisson& gp, const QString& root);

protected:
	static Random random;
};

