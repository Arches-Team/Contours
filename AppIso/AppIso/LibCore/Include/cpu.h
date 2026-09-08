#pragma once

#include <QtWidgets/QGraphicsScene>

class System
{
protected:
  static bool avx; //!< Advanced Vector Extensions flag.
public:
  static QString GetEnv(const QString&);
  static QString GetResource(const QString&, const QString & = QString(""));

  static QString DateTime();

  static void SaveSvg(QGraphicsScene*, const QString&, const QRectF & = QRectF());

  static void FlipVertical(QGraphicsScene&, const QRectF & = QRectF());

  static bool Avx();
};
