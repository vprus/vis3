
#include "canvas.hpp"
#include "trace_model.hpp"
#include "timeline.hpp"
#include "state_model.hpp"
#include "event_model.hpp"
#include "canvas_item.hpp"
#include "group_model.hpp"

#include <QPalette>
#include <QPainter>
#include <QFontMetrics>
#include <QGradient>
#include <QPixmap>
#include <QMouseEvent>
#include <QScrollBar>
#include <QPainterPath>
#include <QKeyEvent>
#include <QPicture>
#include <QWhatsThis>

#include <math.h>
#include <iostream>
using namespace std;

const int arrowhead_length = 16;

// Draws an arrow from (x1, y1) to (x2, y2) on 'painter'.
// The primary issue is that often, there are several arrows
// with the same start position, and zero delta_y. If we draw
// them as straight lines, they will overlap, and look ugly.
// We can't avoid overlap in general, but this case is very
// comment and better be handled.
//
//
// The approach is to make arrows with zero delta y curved.
// When delta y is large enough (or, more specifically, the
// angle to horisontal is small enough, arrow will be drawn as
// straigh line. 
// We compute the angle of straight line between start and end
// to horisontal, and the compute "delta angle" -- angle to that
// straight line that will have start and end segments of the curve.
// For angle of 90 (delta y is zero), delta angle is 30.
// For a "limit angle" of 10, delta angle is 0 (the line is straight)
// For intermediate angles, we interporal.
void draw_unified_arrow(int x1, int y1, int x2, int y2, QPainter& painter,
                        bool always_straight = false, 
                        bool start_arrowhead = false)
{
  // The length of the from the tip of the arrow to the point
    // where line starts.
    const int arrowhead_length = 16;
    
    QPainterPath arrow;
    arrow.moveTo(x1, y1);
    
    // Determine the angle of the straight line.
    double a1 = (x2-x1);
    double a2 = (y2-y1);
    double b1 = 1;
    double b2 = 0;
    
    double straight_length = sqrt(a1*a1 + a2*a2);
    
    double dot_product = a1*b1 + a2*b2;
    double cosine = dot_product/
        (sqrt(pow(a1, 2) + pow(a2, 2))*sqrt(b1 + b2));
    double angle = acos(cosine);
    if (y1 < y2)
    {
        angle = -angle;
    }
    double straight_angle = angle*180/M_PI;
    
    double limit = 10;
    
    double angle_to_vertical;
    if (fabs(straight_angle) < 90)
        angle_to_vertical = fabs(straight_angle);
    else if (straight_angle > 0)
        angle_to_vertical = 180-straight_angle;
    else
        angle_to_vertical = 180-(-straight_angle);
    
    double angle_delta = 0;
    if (!always_straight)
        if (angle_to_vertical > limit)
            angle_delta = 30 * (angle_to_vertical - limit)/90;
    double start_angle = straight_angle > 0 
        ? straight_angle - angle_delta :
        straight_angle + angle_delta;
    
    
    QMatrix m1;
    m1.translate(x1, y1);
    m1.rotate(-start_angle);
    
    double end_angle = straight_angle > 0 
        ? (straight_angle + 180 + angle_delta) :
        (straight_angle + 180 - angle_delta);
    
    QMatrix m2;
    m2.reset();
    m2.translate(x2, y2);        
    m2.rotate(-end_angle);
    
    arrow.cubicTo(m1.map(QPointF(straight_length/2, 0)),              
                  m2.map(QPointF(straight_length/2, 0)),
                  m2.map(QPointF(arrowhead_length, 0)));
    
    painter.save();
    painter.setBrush(Qt::NoBrush);
    painter.drawPath(arrow);    
    painter.restore();


    QPolygon arrowhead(4);
    arrowhead.setPoint(0, 0, 0);
    arrowhead.setPoint(1, arrowhead_length/3, -arrowhead_length*5/4);
    arrowhead.setPoint(2, 0, -arrowhead_length);
    arrowhead.setPoint(3, -arrowhead_length/3, -arrowhead_length*5/4);
    
    painter.save();

    painter.translate(x2, y2);   
    painter.rotate(-90);
    painter.rotate(-end_angle);
    painter.rotate(180);        
    painter.setPen(Qt::black);
    painter.drawPolygon(arrowhead);
    
    painter.restore();            

    painter.save();
    if (start_arrowhead)
    {
        painter.translate(x1, y1);
        
        painter.rotate(-90);
        painter.rotate(-end_angle);
           
        painter.drawPolygon(arrowhead);        
    }
    painter.restore();
}


class Contents_widget : public QWidget
{
public:
    Contents_widget(Canvas* parent);

    void setModel(Trace_model* model);

    void scrolledBy(int dx, int dy);

public: // QWidget overrides


    void paintEvent(QPaintEvent* event);
    void mouseMoveEvent(QMouseEvent* event);

    void mouseDoubleClickEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);

    void resizeEvent(QResizeEvent* event);

    QSize minimumSizeHint() const;
    QSize sizeHint() const;

    QRect boundingRect(int component, int min_time, int max_time);

    Canvas* parent_;
    Trace_model* model_;

    // Qt4 is supposed to do double-buffering by default, but:
    // - in my experiements, it still flickers
    // - we use various overlays (visir, time baloon), and when
    //   they move, we need to restore the content under them.
    //   Using separate pixmap is the simplest way to do this.
    QPixmap* pixmap_;

//    int ballon;
    int visir_position;
    QRect ballon;

    vector<unsigned> lifeline_position;

    QVector<QPair<QRect, int> > clickable_components;
    // FIXME: it might be bad to store all states here,
    // and better solution would be to re-get them from trace
    // on click. Also, possible mem-leak here.
    QVector<QPair<QRect, State_model*> > states;

    std::vector<QRect> lifeline_rects;
    // FIXME: delete all items in destructor.
    std::vector<CanvasItem*> items;

    QCursor mainCursor;

    int pixelPostionForTime(unsigned time);
    int timeForPixel(int pixel_x);
    int lifelineAtPosition(const QPoint& p);


private:

    // Returns a width that is sufficient for drawTextBox, below,
    // to draw text box without clipping.
    int preferredTextBoxWidth(const QString& text);
    
    // Draws text in a nice frame. Uses current pen and brush for 
    // the frame, and black for text.
    // The frame starts and 'x' and is vertically centered around 'y'.
    // The width of frame is 'width', and if text does not fit, it's
    // truncated. If 'width' is -1, width will be auto-computed.
    // If 'text_start_x' is not -1, text is drawing starting
    // on that x position.
    QRect drawTextBox(const QString& text, 
                      QPainter* painter,
                      int x, int y, int width, int height,
                      int text_start_x = -1);

    QRect drawBaloon(QPainter* painter);

    /** Returns the pointer to the number of clickable component
        if point is withing the clickable area, and null otherwise. */
    const int* clickable_component(const QPoint& point) const;

    State_model* clickable_state(const QPoint& point) const;

    bool clickable(const QPoint& point) const;

    void targetUnderCursor(
        const QPoint& pos, Canvas::clickTarget* target,
        int* component, State_model** state, bool* events_near);


public:
    unsigned text_elements_height;
    unsigned text_height;
    unsigned text_letter_width;

    // The width of the area on the left where component
    // names are drawn
    int components_area_width;
    int component_rect_width;
    int right_margin;
    int lifeline_spacing;
    

    QVector< QVector<bool> > eventsNear;

};


Canvas::Canvas(QWidget* parent) : QScrollArea(parent), timeline_(0)
{
    setWhatsThis("<b>Time tigram</b>"
                 "<p>Time diagram shows the components of the distributed system"
                 ", events in those compoments, and states those components are in."
                 "<p>Compoments are green boxes on the left. Composite compoments "
                 "are shown as stack.");

    contents_ = new Contents_widget(this);
    setWidget(contents_);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setWidgetResizable(true);

    timeline_ = new Timeline(this);
    timeline_->setLeftMargin(130);

    setViewportMargins(0, 0, 0, timeline_->sizeHint().height());
}

void Canvas::setModel(Trace_model* model)
{
    emit modelChanged(model);

    contents_->setModel(model);
    timeline_->setTimeRange(model->min_time(), model->max_time());

    contents_->setFocus(Qt::OtherFocusReason);
}

Trace_model* Canvas::model() const
{
    return contents_->model_;
}

void Canvas::setCursor(const QCursor& c)
{
    contents_->setCursor(c);
    contents_->mainCursor = c;
}

int Canvas::nearest_lifeline(int y)
{
    int best_l = -1;
    int best_distance = 10000000;
    for(unsigned i = 0; i < contents_->lifeline_position.size(); ++i)
    {
        int distance = abs((int)(contents_->lifeline_position[i] - y));
        if (distance < best_distance)
        {
            best_distance = distance;
            best_l = i;
        }
    }
    return best_l;
}

QPoint Canvas::lifeline_point(int component, int time)
{
    return QPoint(contents_->pixelPostionForTime(time),
                  contents_->lifeline_position[component]);    
}

QRect Canvas::boundingRect(int component, int min_time, int max_time)
{
    return contents_->boundingRect(component, min_time, max_time);
}

QPair<int, int> Canvas::nearby_range(int time)
{
    int pixel = contents_->pixelPostionForTime(time);
    return qMakePair(contents_->timeForPixel(pixel-20),
                     contents_->timeForPixel(pixel+20));
    
}

void Canvas::scrollContentsBy(int dx, int dy)
{
    contents_->scrolledBy(dx, dy);
    QScrollArea::scrollContentsBy(dx, dy);
}

void Canvas::resizeEvent(QResizeEvent* event)
{
    QScrollArea::resizeEvent(event);

    if (!timeline_)
        return;

    unsigned height = timeline_->sizeHint().height();
    timeline_->resize(viewport()->width(), height);
    timeline_->move(viewport()->x(), viewport()->y() + viewport()->height());
//                    - height);
}

void Canvas::addItem(class CanvasItem* item)
{
    contents_->items.push_back(item);
    item->setParent(contents_);
}

Contents_widget::Contents_widget(Canvas* parent)
: QWidget(parent), parent_(parent), model_(0), pixmap_(0), 
  visir_position((unsigned)-1)
{
    setAttribute(Qt::WA_OpaquePaintEvent, true);
    setAttribute(Qt::WA_NoSystemBackground, true);

    setFocusPolicy(Qt::StrongFocus);

    QFontMetrics fm(font());
    text_elements_height = fm.height() + 2;
    text_height = fm.height();
    text_letter_width = fm.maxWidth();


    QPalette p = palette();
    p.setColor(QPalette::Background, Qt::white);
    p.setColor(QPalette::Foreground, Qt::black);
    setPalette(p);

//    setAutoFillBackground(true);

//    resize(560, 600);
    setSizePolicy(QSizePolicy::Expanding, 
                  QSizePolicy::Expanding);

    setMouseTracking(true);
}

void Contents_widget::setModel(Trace_model* model)
{
    model_ = model;
    clickable_components.clear();
    states.clear();
    lifeline_rects.clear();
    lifeline_position.clear();

    eventsNear.resize(model->component_names().size());
    for(unsigned i = 0; i < model->component_names().size(); ++i)
    {
        eventsNear[i].resize(width());
    }

    QFontMetrics fm(font());

    int lifeline_spacing_factor = 3;
    int em = fm.charWidth("M", 0);
    std::string parent_name = model_->parent_name();
    bool has_parent = !parent_name.empty();

    lifeline_spacing = fm.height()*lifeline_spacing_factor;

    int height = model_->component_names().size()
        * (text_elements_height + lifeline_spacing) - lifeline_spacing;

    if (has_parent)
        height += em + text_elements_height + lifeline_spacing;
                                                   
    pixmap_ = new QPixmap(width(), height);


    right_margin = 0; // It's OK to hardcode, this is just fade-out.
    component_rect_width = 100;
    for (unsigned i = 0; i < model_->component_names().size(); ++i)
    {
        QString name = model_->component_names()[i];
        component_rect_width = max(component_rect_width,
                                   preferredTextBoxWidth(name));
    }
    components_area_width = component_rect_width + em*3;


    QPainter painter(pixmap_);    
    
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::TextAntialiasing);

    painter.fillRect(0, 0, width(), height , Qt::white);

    QLinearGradient g(0, 0, components_area_width, 0);
    g.setColorAt(0, QColor(150, 150, 150));
    g.setColorAt(1, Qt::white);
    painter.setBrush(g);
    painter.setPen(Qt::NoPen);
    painter.drawRect(0, 0, components_area_width, height);            


 
    // Draw the 
    painter.setBrush(QColor("#10e205"));
    painter.setPen(Qt::black);


    unsigned component_start = em;
    unsigned y = em + text_elements_height/2;
    
    int fade_out_width = em;

    painter.setClipRect(0, 0, width()-right_margin, height);

    if (has_parent)
    {
        QRect r = drawTextBox(parent_name.c_str(), &painter, 
                              component_start, y, 
                              component_rect_width + em,
                              text_elements_height);

        clickable_components.push_back(qMakePair(r, -1));

        component_start += em;
        y += lifeline_spacing;
    }

    for(unsigned i = 0; i < model_->component_names().size(); ++i)
    {
        bool composite = model_->has_children(i);
        QString name = model_->component_names()[i];

        if (composite)
        {
            drawTextBox(name, &painter, component_start + 4, y + 4,
                        component_rect_width, text_elements_height);
        }
        QRect r = drawTextBox(name, &painter, component_start, y,
                              component_rect_width, text_elements_height);

        if (composite)
        {
            clickable_components.push_back(qMakePair(r, static_cast<int>(i)));
        }

        lifeline_position.push_back(y);
        painter.drawLine(components_area_width, y, width()-right_margin, y);

        lifeline_rects.push_back(QRect(components_area_width, 
                                       y - text_elements_height/2+1,
                                       width()-right_margin-components_area_width, 
                                       text_elements_height/2*2+2));

        y += lifeline_spacing;

#if 0
        if (i == 3)
        {
            QFont saved(painter.font());
            QFont bold(painter.font());
            bold.setBold(true);
            painter.setFont(bold);
            painter.drawLine(250, ly-10, 250, ly+10);
            painter.drawText(250, ly-10, "C");
            QFont small(painter.font());
            small.setPointSize(8);
            painter.setFont(small);
            painter.drawText(258, ly-6, "u");
            painter.setFont(saved);

        }
#endif        
    }

    painter.setClipRect(components_area_width, 0, 
                        width()-right_margin-components_area_width, height);

    // Draw states.
    model_->rewind();
    for(;;)
    {
        std::auto_ptr<State_model> s = model_->next_state();

        if (!s.get())
            break;

        int pixel_begin = pixelPostionForTime(s->begin);        
        int pixel_end = pixelPostionForTime(s->end);

        painter.setBrush(Qt::yellow);

        int text_begin = -1;
        if (pixel_begin < components_area_width)
        {
            text_begin = components_area_width;
        }

        QRect r = drawTextBox(s->name, &painter, 
                              pixel_begin, lifeline_position[s->component],
                              pixel_end-pixel_begin, text_elements_height,
                              text_begin);

        states.push_back(qMakePair(r, s.release()));
    }

    vector<int> next_available_pos_for_letter(
            model_->component_names().size());
    
    // Draw events.
    model_->rewind();
    for(;;)
    {
        std::auto_ptr<Event_model> e = model_->next_event();

        if (!e.get())
            break;

        int pos = pixelPostionForTime(e->time);        

        unsigned y = lifeline_position[e->component];

        painter.drawLine(pos, y-text_elements_height/2+1, 
                         pos, y+text_elements_height/2+1);

        for(int x = pos-20; x < pos+20 && x < width(); ++x)
        {
            eventsNear[e->component][x] = true;
        }

        if (pos >= next_available_pos_for_letter[e->component])
        {
            QFontMetrics fm(painter.font());
            int w = fm.width(QChar(e->letter));
            next_available_pos_for_letter[e->component] = pos + w;

            painter.drawText(pos, y-text_elements_height/2-1, 
                             QChar(e->letter));
        }
    }

    // Draw groups
    QColor groups_color(Qt::green);
    groups_color.setAlpha(70);    
    painter.setBrush(groups_color);
    for(;;)
    {
        std::auto_ptr<Group_model> g = model_->next_group();

        if (!g.get())
            break;

        if (g->to_component == -1)
            break;

        QPoint from(pixelPostionForTime(g->from_time),
                    lifeline_position[g->from_component]);    

        QPoint to(pixelPostionForTime(g->to_time),
                  lifeline_position[g->to_component]);    

        int delta = text_elements_height/2+2;
        // FIXME: assert that to_component and from_component
        // are not equal.
        if (from.y() < to.y())
        {
            from.setY(from.y() + delta);
            to.setY(to.y() - delta);
        }
        else
        {
            from.setY(from.y() - delta);
            to.setY(to.y() + delta);
        }



        draw_unified_arrow(from.x(), from.y(), to.x(), to.y(), painter);
    }

    // Draw the final fade-out.
    {        
        QLinearGradient g(width()-right_margin-fade_out_width, 
                          0, width()-right_margin, 0);
        g.setColorAt(0, QColor(255, 255, 255, 0));
        g.setColorAt(1, Qt::white);
        
        painter.fillRect(width()-right_margin-fade_out_width, 0, 
                         fade_out_width, height, g);
    }

    

    update();
}

void Contents_widget::paintEvent(QPaintEvent* event)
{
    if (!model_)
        return;
  
    QPainter painter(this);

    painter.drawPixmap(0, 0, *pixmap_);    

    // Draw outside of pixmap
    painter.fillRect(0, pixmap_->height(), width(), 
                     height()-pixmap_->height(), Qt::white);
    QLinearGradient g(0, 0, components_area_width, 0);
    g.setColorAt(0, QColor(150, 150, 150));
    g.setColorAt(1, Qt::white);
    painter.setBrush(g);
    painter.setPen(Qt::NoPen);
    painter.drawRect(0, pixmap_->height(), components_area_width, height()-pixmap_->height());            

    if (visir_position != -1)
    {
        painter.setPen(Qt::red);
        painter.drawLine(visir_position, 0, visir_position, height());

        QColor c("#10e205");
        c.setAlpha(128);
        painter.setBrush(c);
        painter.setPen(Qt::black);

        ballon = drawBaloon(&painter);
    }

    for(unsigned i = 0; i < items.size(); ++i)
    {
        items[i]->draw(painter);
    }
}

void Contents_widget::mouseMoveEvent(QMouseEvent* ev)
{
    if (ev->x() >= components_area_width)
    {
        if (ev->x() != visir_position)
        {
            // Mouse is not on component names area, move visir.
            unsigned old_position = visir_position;
            visir_position = ev->x();

            update(QRect(old_position, 0, 1, height()) |
                   QRect(visir_position, 0, 1, height()));
            
            update(ballon);
            update(drawBaloon(0)); 
        }
    }

    Canvas::clickTarget target = Canvas::nothingClicked;
    int component = -1;
    State_model* state = 0;
    bool events_near = false;
    
    targetUnderCursor(ev->pos(), &target, &component, &state, &events_near);
    
    emit parent_->mouseMoveEvent(ev, target, 
                                 lifelineAtPosition(ev->pos()),
                                 timeForPixel(ev->x()));
}

void Contents_widget::scrolledBy(int dx, int dy)
{
    update(ballon);
    ballon.adjust(0, -dy, 0, -dy);
    update(ballon);
    update();
}

QSize Contents_widget::minimumSizeHint() const
{
    if (!model_)
        return QSize(300, 200);
    else
        return QSize(300, model_->component_names().size()*lifeline_spacing 
                     + text_elements_height + 100);
}


QSize Contents_widget::sizeHint() const
{
    return minimumSizeHint();
}

int Contents_widget::preferredTextBoxWidth(const QString& text)
{
    QFontMetrics fm(font());
    int width = fm.width(text);
    
    int left_right_pad = fm.charWidth("n", 0);
    width += 2*left_right_pad;    

    return width;
}


QRect Contents_widget::drawTextBox(
    const QString& text, QPainter* painter,
    int x, int y, int width, int height,
    int text_start_x)
{    
    if (width == -1)
    {
        width = preferredTextBoxWidth(text);
    }

    QRect r(x, y-height/2, width, height);

    if (painter)
        painter->drawRect(r);

    QFontMetrics fm(font());
    int left_right_pad = fm.charWidth("n", 0);

    QRect text_r(r);
    text_r.adjust(left_right_pad, 0, -left_right_pad, 0);

    if (text_start_x != -1)
    {
        text_r.setLeft(text_start_x);
    }

    if (painter)
        painter->drawText(text_r, Qt::AlignLeft|Qt::AlignVCenter, text);

    // Note: when calling with null QPainter to get just metrics,
    // we can't be sure that pen width will be 1 in subsequent calls.
    // Have to trust the user.
    int pen_width = 1;
    r.adjust(-pen_width, -pen_width, pen_width, pen_width);

    return r;
}

QRect Contents_widget::drawBaloon(QPainter* painter)
{
    QString s = QString::number(timeForPixel(visir_position));
    int x = visir_position + 10;
    int y = parent_->verticalScrollBar()->value() + 15;

    QRect try_on_right = drawTextBox(s, 0, x, y, -1, text_elements_height);
    if (try_on_right.right() > width()-right_margin)
    {
        // Draw on left.
        return drawTextBox(s, painter, 
                           visir_position - 10 - try_on_right.width(),
                           y, -1, text_elements_height);
    }
    else
    {
        if (painter)
            drawTextBox(s, painter, x, y, -1, text_elements_height);
        return try_on_right;
    }
}


int Contents_widget::pixelPostionForTime(unsigned time)
{
    int lifelines_width = pixmap_->width() - components_area_width - right_margin;

    int delta = (int)time - (int)model_->min_time();
    int max = (int)(model_->max_time()) - (int)(model_->min_time());

    double ratio = double(delta)/double(max);
    return int(ratio*lifelines_width + components_area_width);
}

int Contents_widget::timeForPixel(int pixel_x)
{
    unsigned lifelines_width = pixmap_->width() - components_area_width 
        - right_margin;
    pixel_x -= components_area_width;

    return int(double(pixel_x)/lifelines_width
        *(model_->max_time()-model_->min_time()) + model_->min_time());
}


QRect Contents_widget::boundingRect(int component, int min_time, int max_time)
{
    int left = pixelPostionForTime(min_time);
    int right = pixelPostionForTime(max_time);

    int y_mid = lifeline_position[component];

    // The event line is text_element_height/2
    // The letter is drawn one pixel above and can take
    // as much as text_height.
    int y_delta = text_elements_height/2 + 1 + text_height;
    std::cout << "text_letter_width " << text_letter_width << "\n";
    left -= text_letter_width;
    right += text_letter_width;

    int y_top = y_mid - y_delta;
    int y_bottom = y_mid + y_delta;

    return QRect(QPoint(left, y_top), QPoint(right, y_bottom));
}


const int* Contents_widget::clickable_component(const QPoint& point) const
{    
    typedef QPair<QRect, int> pt;
    foreach(const pt& p, clickable_components)
    {
        if (p.first.contains(point))
            return &p.second;
    }
    return 0;
}

State_model* Contents_widget::clickable_state(const QPoint& point) const
{
    typedef QPair<QRect, State_model*> pt;
    foreach(const pt& p, states)
    {
        if (p.first.contains(point))
            return p.second;
    }
    return 0;
}

bool Contents_widget::clickable(const QPoint& point) const
{
    return clickable_component(point) || clickable_state(point);
}

void Contents_widget::mouseDoubleClickEvent(QMouseEvent* event)
{    
    QWidget::mouseDoubleClickEvent(event);
}

int Contents_widget::lifelineAtPosition(const QPoint& p)
{
    int component = -1;
    for(unsigned i = 0; i < lifeline_rects.size(); ++i)
    {
        if (lifeline_rects[i].contains(p))
        {
            component = i;
        }
    }    
    return component;
}

void Contents_widget::targetUnderCursor(
    const QPoint& pos, Canvas::clickTarget* target,
    int* component, State_model** state, bool* events_near)
{
    *target = Canvas::nothingClicked;

    if (pos.x() < components_area_width)
    {
        const int* c = clickable_component(pos);
        if (c)
        {
            if (*c == -1)
            {
                *target = Canvas::rootClicked;
            }
            else
            {
                *target = Canvas::componentClicked;
                *component = *c;
            }
        }
    }
    else
    {
        *state = clickable_state(pos);

        if (*state)
        {
            *target = Canvas::stateClicked;
        }
        else
        {
            *target = Canvas::lifelinesClicked;
        }

        *component = lifelineAtPosition(pos);
        if (*component != -1)
        {
            if (eventsNear[*component][pos.x()])
            {
                *events_near = true;
            }
        }  
    }    
}

void Contents_widget::mouseReleaseEvent(QMouseEvent* event)
{
    QWidget::mouseReleaseEvent(event);

    Canvas::clickTarget target;
    int component = -1;
    State_model* state = 0;
    bool events_near = false;

    targetUnderCursor(event->pos(), &target, &component, &state, &events_near);

    emit parent_->mouseEvent(event, 
                             target,
                             component,
                             state,
                             timeForPixel(event->x()),
                             events_near);
}

void Contents_widget::resizeEvent(QResizeEvent* event)
{
    visir_position = -1;
    setModel(model_);
}



