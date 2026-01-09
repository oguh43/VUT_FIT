#ifndef STATE_ELLIPSE_ITEM_H
#define STATE_ELLIPSE_ITEM_H

#include <QGraphicsEllipseItem>
#include <QGraphicsSceneMouseEvent>
#include <QCursor>
#include <QObject>

class state_ellipse_item : public QObject, public QGraphicsEllipseItem
{
    Q_OBJECT

public:
    state_ellipse_item(const QRectF& rect, QGraphicsItem* parent = nullptr);

signals:
    void stateMoved();  // We'll use this to notify the editor

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
};

#endif // STATE_ELLIPSE_ITEM_H
