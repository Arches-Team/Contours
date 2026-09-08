// System

#include <QtGui/QPainter>

#include <QtSvg/QSvgGenerator>
#include <QtWidgets/QGraphicsItem>

#include "cpu.h"

//   // Use the following code to save as SVG
//   /*
//   //#include <QtSvg/QSvgGenerator>

void System::SaveSvg(QGraphicsScene* s, const QString& name, const QRectF& rect)
{
  /*
  QRectF newSceneRect;
  QGraphicsScene* tempScene = new QGraphicsScene(s->sceneRect());
  tempScene->setBackgroundBrush(QBrush(Qt::transparent));
  tempScene->setItemIndexMethod(QGraphicsScene::BspTreeIndex);

  foreach(QGraphicsItem * item, s->items())
  {
    newSceneRect |= item->mapToScene(item->boundingRect()).boundingRect();
    tempScene->addItem(item);
  }
  tempScene->setSceneRect(newSceneRect);
  tempScene->clearSelection();
  QSize sceneSize = newSceneRect.size().toSize();
  */
  QRectF sceneBox;
  if (rect.isNull())
    sceneBox = s->sceneRect();
  else
    sceneBox = rect;

  QSize sceneSize = sceneBox.size().toSize();
  QRectF targetBox(0, 0, sceneSize.width(), sceneSize.height());

  QSvgGenerator generator;
  generator.setFileName(name);
  generator.setSize(sceneSize);
  generator.setViewBox(targetBox);
  generator.setDescription(QObject::tr("SVG Export"));
  generator.setTitle(name);

  QPainter painter;
  painter.begin(&generator);
  s->render(&painter, targetBox, sceneBox, Qt::IgnoreAspectRatio);
  painter.end();

  //tempScene->clear();
  //delete tempScene;
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

