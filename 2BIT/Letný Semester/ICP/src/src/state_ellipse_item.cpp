#include "../headers/state_ellipse_item.h"

state_ellipse_item::state_ellipse_item(const QRectF& rect, QGraphicsItem* parent)
    : QGraphicsEllipseItem(rect, parent)
{
    setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemSendsScenePositionChanges);
    setCursor(Qt::OpenHandCursor);
}

void state_ellipse_item::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        setCursor(Qt::ClosedHandCursor);
    }
    QGraphicsEllipseItem::mousePressEvent(event);
}

void state_ellipse_item::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    QGraphicsEllipseItem::mouseMoveEvent(event);
    emit stateMoved();  // Notify that the item moved
}

void state_ellipse_item::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    setCursor(Qt::OpenHandCursor);
    QGraphicsEllipseItem::mouseReleaseEvent(event);
}
