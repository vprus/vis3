
#include "tool.hpp"

#include "canvas_item.hpp"
#include "canvas.hpp"
#include "trace_model.hpp"
#include "event_model.hpp"
#include "selection_widget.hpp"

#include <QAction>
#include <QPainter>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QShortcut>


class Found_item_highlight : public CanvasItem
{
public:
    void setRect(QRect r)
    {
        rect = r;
        if (rect.isValid())
        {
            r.adjust(-2, -2, 2, 2);
        }
        new_geomerty(r);            
    }
    
private:
    QRect rect;
         
private:
    QRect xdraw(QPainter& painter)
    {
        if (!rect.isValid())
            return QRect();

        painter.save();
        
        QColor halfRed(Qt::red);
        halfRed.setAlpha(75);
        painter.setBrush(halfRed);
        painter.setPen(QPen(Qt::red, 2));

        painter.drawRect(rect);
        QRect r = rect;
        r.adjust(-2, -2, 2, 2);

        painter.restore();
        
        return r;        
    }
};

class Find : public Tool
{
    Q_OBJECT
public:
    Find(QWidget* parent, Canvas* c) 
    : Tool(parent, c)
    {
        setWindowTitle("Find");


        QAction* find_again = new QAction(parent);
        find_again->setShortcut(Qt::Key_F3);
        find_again->setShortcutContext(Qt::ApplicationShortcut);
        parent->addAction(find_again);

        connect(find_again, SIGNAL(triggered(bool)),
                this, SLOT(findNextEvent()));


        QVBoxLayout* mainLayout = new QVBoxLayout(this);

#if 0

        QGroupBox* kind = new QGroupBox("Items to find", this);
        mainLayout->addWidget(kind);

        QVBoxLayout* kindLayout = new QVBoxLayout(kind);

        QRadioButton* findEvents = new QRadioButton("Events", kind);
        kindLayout->addWidget(findEvents);
        QRadioButton* findStates = new QRadioButton("States", kind);
        kindLayout->addWidget(findStates);
#endif

        selector = new SelectionWidget("Events to find", this);
        mainLayout->addWidget(selector);

        mainLayout->addStretch();

        QHBoxLayout* buttons = new QHBoxLayout(0);
        mainLayout->addLayout(buttons);

        buttons->addStretch();
        QPushButton* findButton = new QPushButton("Find", this);
        buttons->addWidget(findButton);



        highlight = new Found_item_highlight;
        canvas()->addItem(highlight);

        connect(findButton, SIGNAL(clicked(bool)),
                this, SLOT(startFind()));

//        connect(select, SIGNAL(selectionChanged(const QVector<bool>&)),
//                this, SLOT(selectionChanged(const QVector<bool>&)));
                                  //mid_time, mid_time));

        connect(canvas(), SIGNAL(modelChanged(Trace_model*)),
                this, SLOT(modelChanged(Trace_model*)));

        QShortcut* sk = new QShortcut(Qt::Key_Enter, this);
        connect(sk, SIGNAL(activated()),
                findButton, SLOT(animateClick()));

        filtered = model();
    }

    QAction* createAction()
    {
        QAction* find_action = new QAction(QIcon(":/find.png"), "&Find", 
                                             this);
        find_action->setObjectName("find_action");
        find_action->setShortcut(QKeySequence(Qt::Key_F));
        return find_action;
    }

    void activate() 
    {
        /* FIXME: remember the previous selection. */
        selector->events_setModel(model(), filtered);
    }
    void deactivate() {}

signals:
    void extraHelp(const QString& s);

private slots:
    void selectionChanged(const QVector<bool>& new_selection)
    {
        filtered = filtered->unfiltered()->filter_events(new_selection);
    }

    void startFind()
    {
        filtered = 
            filtered->set_range(model()->min_time(), 
                                filtered->root()->max_time());
        filtered->rewind();

        findNextEvent();
    }



    void findNextEvent()
    {
        std::auto_ptr<Event_model> e = filtered->next_event();

        if (e.get())
        {
            unsigned event_time = e->time;

            if (event_time > model()->max_time())
            {
                int shown_time_range = model()->max_time() 
                    - model()->min_time();                

                int one_tenth = shown_time_range/10;

                int desired_new_min_time = event_time - one_tenth;
                unsigned new_max_time = desired_new_min_time + shown_time_range;
                if (new_max_time > model()->root()->max_time())
                {
                    new_max_time = model()->root()->max_time();
                }
                int new_min_time = new_max_time - shown_time_range;

                canvas()->setModel(
                    model()->set_range(new_min_time, new_max_time));
            }
            

            QRect r = canvas()->boundingRect(e->component,
                                             e->time, e->time);

            r.adjust(-1, -1, 1, 1);
            highlight->setRect(r);

            emit showEvent(e.release());
            emit extraHelp("<b>Press F3 for next event</b>");
        }
        else
        {
            highlight->setRect(QRect());
            emit extraHelp(QString());
        }
    }

    void modelChanged(Trace_model*)
    {
        highlight->setRect(QRect());
        emit extraHelp(QString());
        
    }


private:
    Trace_model* filtered;

    SelectionWidget* selector;

    Found_item_highlight* highlight;
};


Tool* createFind(QWidget* parent, Canvas* canvas)
{
    return new Find(parent, canvas);
}

#include "find.moc"
