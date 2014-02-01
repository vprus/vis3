
#ifndef TIMELINE_HPP_VP_2006_03_21
#define TIMELINE_HPP_VP_2006_03_21

#include <QWidget>

class Timeline : public QWidget
{
public:
    Timeline(QWidget* parent);
    
    void setTimeRange(unsigned min, unsigned max);
    
    void setLeftMargin(unsigned pixels);
    void selRightMargin(unsigned pixels);

public: // QWidget overides

    QSize sizeHint() const;
    QSize minimumSizeHint() const;


protected:
    void paintEvent(QPaintEvent* e);

    unsigned min_time_;
    unsigned max_time_;
    unsigned left_margin_;
    unsigned right_margin_;
};

#endif
