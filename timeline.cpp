
#include "timeline.hpp"

#include <QPainter>
#include <QPolygon>


// The height of markers
const int text_top = 13;


Timeline::Timeline(QWidget* parent)
: QWidget(parent), min_time_(0), max_time_(100), left_margin_(5),
  right_margin_(5)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
}
    
void Timeline::setTimeRange(unsigned min, unsigned max)
{
    min_time_ = min;
    max_time_ = max;
    update();
}
    
void Timeline::setLeftMargin(unsigned pixels)
{
    left_margin_ = pixels;
    update();
}

void Timeline::selRightMargin(unsigned pixels)
{
    right_margin_ = pixels;
    update();
}

QSize Timeline::sizeHint() const
{
    return QSize(-1, text_top + QFontMetrics(font()).height());
}

QSize Timeline::minimumSizeHint() const
{
    return sizeHint();
}


void Timeline::paintEvent(QPaintEvent* e)
{
    QPainter p(this);

    p.fillRect(geometry(), QColor(255, 255, 255, 100));

    p.setBrush(Qt::black);
    p.setPen(Qt::black);

    p.drawLine(0, 0, width()-1, 0);

    p.setRenderHint(QPainter::Antialiasing);    

    unsigned pixel_lenth = width()-right_margin_-left_margin_;
    unsigned time_width = max_time_ - min_time_ + 1;

    int max = width()-right_margin_;
    for(int pos = left_margin_; pos < max; pos += 5)
    {
        if (((pos-left_margin_) % 100) == 0)
        {
            QPolygon arrow(3);
            arrow.setPoint(0, pos, 2+2);
            arrow.setPoint(1, pos+3, 9+2);            
            arrow.setPoint(2, pos-3, 9+2);            
            p.drawPolygon(arrow);
            

            unsigned time_here = int(double(pos-left_margin_)
                /pixel_lenth*time_width + min_time_);

            QString lti = QString::number(time_here);

            QFontMetrics fm(p.font());
            int width = fm.width(lti);
            
            p.drawText( pos-width/2, text_top, width, 100, 
                        Qt::AlignTop | Qt::AlignHCenter, lti);
        }
        else if (((pos-left_margin_) % 10) == 0)
        {            
            p.drawLine(pos, 3+2, pos, 6+2);
        }
    }
}
