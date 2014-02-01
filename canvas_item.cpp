
#include "canvas_item.hpp"

#include <QWidget>

CanvasItem::CanvasItem()
{}


void CanvasItem::draw(QPainter& painter)
{
    currentRect = xdraw(painter);
}

void CanvasItem::new_geomerty(const QRect& r)
{
    static_cast<QWidget*>(parent())->update(r | currentRect);
}
