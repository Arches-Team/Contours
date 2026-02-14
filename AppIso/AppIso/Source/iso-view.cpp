#include "iso-view.h"
#include "draw.h"

#include <QtWidgets/QScrollbar>

using namespace std;

IsoView::IsoView(QWidget* parent) : QGraphicsView(parent), radiusInMeters(1000), showCircle(false), isDrawing(false), showPath(false), showProtectedZone(false)
{
	// Sans ça, impossible de suivre le curseur si aucun bouton n'est appuyé.
	setMouseTracking(true);
    setDragMode(QGraphicsView::ScrollHandDrag);

    if (scene() == nullptr)
    {
        setScene(new QGraphicsScene);
    }
}

void IsoView::ChangeIsos(IsoLines* newIsos)
{
    isos = newIsos;
    DisplayIsos();
}

void IsoView::ClearView()
{
    ClearPath();
    ClearIsos();
    ClearBackgound();
    ClearProtectedZones();
    scene()->clear();
}

void IsoView::ResetView()
{
    QRectF rect = scene()->sceneRect();
    if (sceneBackgroundItem != nullptr)
        rect = sceneBackgroundItem->rect();
    resetTransform();  // Clear previous scale, zoom, rotate, etc.
    fitInView(rect, Qt::KeepAspectRatio);
    scale(1, -1); // Pour que l'image soit à l'endroit, car Qt met les y vers le bas et pas vers le haut comme nous
}

void IsoView::ResetViewRect()
{
    if (isos == nullptr)
        return;

    Box2 b = isos->GetBox();
    b.Scale(1.01);
    SetViewRect(b);
}

void IsoView::SetViewRect(const Box2& b)
{
    ClearBackgound();
    sceneBackgroundItem = scene()->addRect(b.GetQtRect(), QPen(Qt::white), QBrush(Qt::white));
    sceneBackgroundItem->setZValue(-9999);

    // Force scene rect to match the background
    scene()->setSceneRect(b.GetQtRect());
}

Box2 IsoView::ViewRect() const
{
    QRectF r;

    if (scene() == nullptr)
    {
        return Box2();
    }
    if (sceneBackgroundItem == nullptr)
    {
        r = scene()->itemsBoundingRect();
    }
    else
    {
        r = sceneBackgroundItem->rect();
    }

    Vector2 a(r.left(), r.top());
    Vector2 b(r.right(), r.bottom());
    return Box2(a, b);
}

void IsoView::SetProtectedZones(QVector<Circle2>* zones)
{
    protectedZones = zones;
    if (showProtectedZone)
        DisplayProtectedZones();
}

QSet<int> IsoView::GetSelectedIsos() const
{
    QSet<int> selected;

    for (QGraphicsItem* item : scene()->selectedItems())
        if (IsoLineItem* iso = qgraphicsitem_cast<IsoLineItem*>(item))
            selected.insert(iso->GetIsoLineInd());

    return selected;
}

void IsoView::ClearIsos()
{
    for (QGraphicsItem* item : isosItems)
    {
        scene()->removeItem(item);
        delete item;
    }
    isosItems.clear();
}

void IsoView::ClearPath()
{
    for (QGraphicsItem* item : pathItems)
    {
        scene()->removeItem(item);
        delete item;
    }
    pathItems.clear();
}

void IsoView::ClearBackgound()
{
    if (sceneBackgroundItem != nullptr)
    {
        scene()->removeItem(sceneBackgroundItem);
        delete sceneBackgroundItem;
        sceneBackgroundItem = nullptr;
    }
}

void IsoView::ClearProtectedZones()
{
    if (protectedZoneItem != nullptr)
    {
        scene()->removeItem(protectedZoneItem);
        delete protectedZoneItem;
        protectedZoneItem = nullptr;
    }
}

void IsoView::DisplayIsos()
{
    ClearIsos();

    if (isos == nullptr || isos->Size() == 0)
    {
        viewport()->update();
        return;
    }

    for (int i = 0; i < isos->Size(); ++i)
    {
        IsoLineItem* isoItem = new IsoLineItem(isos, i);
        scene()->addItem(isoItem);
        isosItems.append(isoItem);
    }
    viewport()->update();
}

void IsoView::DisplayCurrentPath()
{
    QGraphicsScene* s = scene();
    if (!s)
        return;

    ClearPath();
    if (pathPoints.size() < 2)
        return;

    QPen linePen(Qt::red);
    linePen.setWidthF(2);
    linePen.setCosmetic(true);

    for (int i = 0; i < pathPoints.size(); ++i)
    {
        QPoint pt(pathPoints[i][0], pathPoints[i][1]);

        // Draw red line to next point
        if (i < pathPoints.size() - 1)
        {
            QPoint pt2(pathPoints[i + 1][0], pathPoints[i + 1][1]);
            QGraphicsLineItem* lineItem = s->addLine(QLineF(pt, pt2), linePen);
            pathItems.append(lineItem);
        }
    }
}

void IsoView::DisplayProtectedZones()
{
    QGraphicsScene* s = scene();
    if (!s || !protectedZones)
        return;

    ClearProtectedZones();
    
    QPainterPath path;
    path.setFillRule(Qt::WindingFill);  // ensure union behavior

    for (int i = 0; i < protectedZones->size(); ++i)
    {
        Vector2 c = protectedZones->at(i).Center();
        double r = protectedZones->at(i).Radius();

        path.addEllipse(c[0] - r, c[1] - r, r + r, r + r);
    }
    
    auto item = new QGraphicsPathItem(path);
    item->setBrush(QColor(255, 0, 0, 100));
    item->setPen(Qt::NoPen);
    s->addItem(item);

    protectedZoneItem = item;
}

void IsoView::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::RightButton)
    {
        pathPoints.clear();
        isDrawing = true;
        QPointF scenePos = mapToScene(event->pos());
        Vector2 point = Vector2(scenePos.x(), scenePos.y());
        emit rightClickOn(point);
        pathPoints.append(point);
    }

    QGraphicsView::mousePressEvent(event);
}

void IsoView::mouseMoveEvent(QMouseEvent* event)
{
    QPoint mousePos = event->pos();
    mouseScenePos = mapToScene(mousePos);

    if (isDrawing)
    {
        Vector2 point = Vector2(mouseScenePos.x(), mouseScenePos.y());
        emit rightClickOn(point);
        pathPoints.append(point);
        if (showPath)
            DisplayCurrentPath();
    }

    viewport()->update();
    QGraphicsView::mouseMoveEvent(event); // <--- This is required for dragmode
}

void IsoView::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::RightButton)
    {
        isDrawing = false;
        ClearPath();
        emit pathFinished(pathPoints);
    }

    QGraphicsView::mouseReleaseEvent(event);
}

//void IsoView::enterEvent(QEnterEvent* event)
//{
//    Q_UNUSED(event);
//    viewport()->update();
//}
//
//void IsoView::leaveEvent(QEvent* event)
//{
//    Q_UNUSED(event);
//    viewport()->update();
//}

void IsoView::drawForeground(QPainter* painter, const QRectF& rect)
{
    Q_UNUSED(rect);
    if (!showCircle) return;

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setPen(QPen(Qt::red, 0)); // 0 width = cosmetic pen

    // Radius in scene coordinates
    qreal radius = radiusInMeters;

    painter->drawEllipse(mouseScenePos, radius, radius);
    painter->restore();
}

void IsoView::wheelEvent(QWheelEvent* event)
{
    if (scene() == nullptr)
        return;

    // Holding Alt change the axis of the angle
    const int delta = Qt::AltModifier & QApplication::keyboardModifiers() ? event->angleDelta().x() : event->angleDelta().y();

    // Scroll up/down
    if (Qt::NoModifier == QApplication::keyboardModifiers())
    {
        int value = (delta < 0) ? 50 : -50;
        verticalScrollBar()->setValue(verticalScrollBar()->value() + value);
    }
    // Scroll left/right
    else if (Qt::ShiftModifier == QApplication::keyboardModifiers())
    {
        int value = (delta < 0) ? 50 : -50;
        horizontalScrollBar()->setValue(horizontalScrollBar()->value() + value);
    }
    // Zoom/dezoom
    else if (Qt::ControlModifier == QApplication::keyboardModifiers())
    {
        float factor = (delta < 0) ? 0.5 : 2.0;
        scale(factor, factor);

        // To zoom on the mouse
        QPointF deltaViewportPos = event->position() - QPointF(viewport()->width() / 2.0, viewport()->height() / 2.0);
        QPointF viewportCenter = mapFromScene(mouseScenePos) - deltaViewportPos;
        centerOn(mapToScene(viewportCenter.toPoint()));
    }
    // Modification radius
    else if (Qt::AltModifier == QApplication::keyboardModifiers())
    {
        float factor = (delta < 0) ? 0.5 : 2.0;
        emit radiusChanged(radiusInMeters * factor);
        viewport()->update();
    }

    // Change mouse position to keep circle around the mouse
    QPointF mousePos = event->position();
    mouseScenePos = mapToScene(mousePos.toPoint());

    // Change bounding rect of the view so the user can go outside of the scene but not too much
    QRectF bounds = scene()->itemsBoundingRect();
    QRectF viewRect = mapToScene(viewport()->rect()).boundingRect();

    const qreal marginX = viewRect.width() / 2.0;
    const qreal marginY = viewRect.height() / 2.0;

    QRectF paddedSceneRect = bounds.adjusted(-marginX, -marginY, marginX, marginY);
    scene()->setSceneRect(paddedSceneRect);
}

void IsoView::keyPressEvent(QKeyEvent* event)
{
    emit keyPressed(event->key());
    QGraphicsView::keyPressEvent(event);
}

FixedSizePointItem::FixedSizePointItem(const QPointF& p, qreal r) : position(p), radius(r)
{
    setPos(p);
    setFlag(QGraphicsItem::ItemIgnoresTransformations);
}

void FixedSizePointItem::SetOutline(qreal s, const QColor& c)
{
    outlineSize = s;
    outlineColor = c;
}

void FixedSizePointItem::SetColor(const QColor& c)
{
    color = c;
}

void FixedSizePointItem::SetRadius(qreal r)
{
    radius = r;
}

IsoLineItem::IsoLineItem(IsoLines* isos, int i) : QGraphicsPolygonItem(isos->At(i).GetQt()), isos(isos), ind(i)
{
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setAcceptHoverEvents(true);

    AnalyticPalette palette(3);
    
    double min;
    double max;
    isos->GetRange(min, max);

    double outT = (max - min) == 0 ? 1 : (isos->At(i).H() - min) / (max - min);
    double inT = (max - min) == 0 ? 1 : (isos->HeightInside(i) - min) / (max - min);
    outCol = palette.GetColor(outT).GetQt();
    inCol = palette.GetColor(inT).GetQt();
}

#include <QtWidgets/QStyleOptionGraphicsItem>
void IsoLineItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    QPen p;
    p.setCosmetic(true);
    p.setWidth(2);
    p.setColor(outCol);

    if (option->state & QStyle::State_Selected)
    {
        p.setWidth(3);
        setBrush(QColor(inCol.red(), inCol.green(), inCol.blue(), 75.5));
    }
    else if (option->state & QStyle::State_MouseOver)
    {
        p.setWidth(3);
        setBrush(QColor(inCol.red(), inCol.green(), inCol.blue(), 75.5));
    }
    else
    {
        setBrush(Qt::NoBrush);
    }

    // To only draw inside the polygon but not inside children
    QPainterPath path;
    path.addPolygon(polygon());
    for (int c : isos->Children(ind))
    {
        QPainterPath hole;
        hole.addPolygon(isos->At(c).GetQt());
        path = path.subtracted(hole);
    }
    path.closeSubpath();

    setPen(p);

    painter->setPen(pen());
    painter->setBrush(brush());
    painter->drawPath(path);
}

QRectF FixedSizePointItem::boundingRect() const
{
    qreal r = radius;
    return QRectF(-r, -r, 2 * r, 2 * r);
}

void FixedSizePointItem::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
{
    if (outlineSize > 0)
    {
        QPen pen(outlineColor);
        pen.setWidth(outlineSize);
        painter->setPen(pen);
    }
    else
    {
        painter->setPen(Qt::NoPen);
    }
    painter->setBrush(color);
    painter->drawEllipse(QPointF(0, 0), radius, radius);
}
