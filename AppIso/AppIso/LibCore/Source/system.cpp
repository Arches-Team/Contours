// System

#include <QtGui/QPainter>

#include <QtSvg/QSvgGenerator>
#include <QtWidgets/QGraphicsItem>

#include "cpu.h"

#include <algorithm>

// Largest side (in declared SVG units) the exported file is allowed to claim as its
// physical width/height. A terrain box tens of kilometers wide would otherwise produce
// an SVG declaring itself tens of thousands of units in size, which most viewers either
// choke on or refuse to render. The viewBox keeps the full coordinate range, so the
// vector content itself loses no precision, only the declared physical size is capped.
static const int MaxSvgDeclaredSize = 2048;

void System::SaveSvg(QGraphicsScene* s, const QString& name, const QRectF& rect)
{
  QRectF sceneBox;
  if (rect.isNull())
    sceneBox = s->sceneRect();
  else
    sceneBox = rect;

  QSize sceneSize = sceneBox.size().toSize();
  QRectF targetBox(0, 0, sceneSize.width(), sceneSize.height());

  QSize outputSize = sceneSize;
  int largestSide = std::max(outputSize.width(), outputSize.height());
  if (largestSide > MaxSvgDeclaredSize)
  {
    double scale = double(MaxSvgDeclaredSize) / double(largestSide);
    outputSize = QSize(std::max(1, int(outputSize.width() * scale)), std::max(1, int(outputSize.height() * scale)));
  }

  QSvgGenerator generator;
  generator.setFileName(name);
  generator.setSize(outputSize);
  generator.setViewBox(targetBox);
  generator.setDescription(QObject::tr("SVG Export"));
  generator.setTitle(name);

  QPainter painter;
  painter.begin(&generator);
  s->render(&painter, targetBox, sceneBox, Qt::IgnoreAspectRatio);
  painter.end();
}

/*!
\brief Flip all elements in the scene vertically.
\param scene The scene.
\param rect The rectangle which gives the symmetry line
*/
void System::FlipVertical(QGraphicsScene& scene, const QRectF& rect)
{
  QRectF bounds;
  if (rect.isNull())
    bounds = scene.itemsBoundingRect();
  else
    bounds = rect;
  qreal centerY = bounds.center().y();

  for (QGraphicsItem* item : scene.items())
  {
    QPointF pos = item->pos();
    qreal newY = 2 * centerY - pos.y();
    item->setPos(pos.x(), newY);

    QTransform trans = item->transform();
    trans.scale(1, -1);
    item->setTransform(trans);
  }
}

