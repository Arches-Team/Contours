#pragma once

#include <QtWidgets/QGraphicsView>
#include <QtWidgets/QAbstractGraphicsShapeItem>
#include "iso-line.h"

class IsoView : public QGraphicsView
{
	Q_OBJECT

protected:
	// The background rect of the scene
	QGraphicsRectItem* sceneBackgroundItem = nullptr;

	// To handle isos
	IsoLines* isos = nullptr;
	QVector<QGraphicsItem*> isosItems;

	// To display circle
	double radiusInMeters;
	bool showCircle;
	QPointF mouseScenePos;

	// To draw a path
	bool isDrawing;
	bool showPath;
	QVector<Vector2> pathPoints;
	QVector<QGraphicsItem*> pathItems;

	// To Display protected Zones
	bool showProtectedZone;
	QVector<Circle2>* protectedZones = nullptr;
	QGraphicsItem* protectedZoneItem = nullptr;

public:
	IsoView(QWidget* = nullptr);

	double Radius() const;
	void SetRadius(double);
	void DisplayCircle(bool);
	void DisplayPath(bool);
	void DisplayProtectedZones(bool);

	void ChangeIsos(IsoLines*);
	void ClearView();
	void ResetView();
	void ResetViewRect();
	void SetViewRect(const Box2&);
	Box2 ViewRect() const;

	void SetProtectedZones(QVector<Circle2>*);
	QSet<int> GetSelectedIsos() const;

protected:
	void ClearIsos();
	void ClearPath();
	void ClearBackgound();
	void ClearProtectedZones();

	void DisplayIsos();
	void DisplayCurrentPath();
	void DisplayProtectedZones();

signals:
	void rightClickOn(const Vector2&);
	void pathFinished(const QVector<Vector2>&);
	void radiusChanged(double);
	void keyPressed(int);

protected:
	void mousePressEvent(QMouseEvent*) override;
	void mouseMoveEvent(QMouseEvent*) override;
	void mouseReleaseEvent(QMouseEvent*) override;
	//void enterEvent(QEnterEvent*) override;
	//void leaveEvent(QEvent*) override;
	void drawForeground(QPainter*, const QRectF&) override;
	void wheelEvent(QWheelEvent*) override;
	void keyPressEvent(QKeyEvent* event) override;
};

inline double IsoView::Radius() const
{
	return radiusInMeters;
}

inline void IsoView::SetRadius(double r)
{
	radiusInMeters = r;
}

inline void IsoView::DisplayCircle(bool show)
{
	showCircle = show;
}

inline void IsoView::DisplayPath(bool show)
{
	showPath = show;
}

inline void IsoView::DisplayProtectedZones(bool show)
{
	if (show == showProtectedZone)
		return;
	showProtectedZone = show;
	if (show)
		DisplayProtectedZones();
	else
		ClearProtectedZones();
}

class IsoLineItem : public QGraphicsPolygonItem
{
protected:
	IsoLines* isos;
	int ind;
	QColor outCol;
	QColor inCol;

public:
	IsoLineItem(IsoLines*, int);
	int GetIsoLineInd() const { return ind; }

	void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*) override;
};

class FixedSizePointItem : public QGraphicsItem
{
protected:
	QPointF position;
	qreal radius;
	qreal outlineSize = 0;
	QColor color = Qt::green;
	QColor outlineColor = Qt::black;

public:
	FixedSizePointItem(const QPointF& position, qreal = 5);

	void SetOutline(qreal, const QColor & = Qt::black);
	void SetColor(const QColor&);
	void SetRadius(qreal);

	QRectF boundingRect() const override;
	void paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) override;
};
