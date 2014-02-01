
#include "tool.hpp"

#include "trace_model.hpp"
#include "event_model.hpp"
#include "state_model.hpp"
#include "canvas.hpp"

#include "event_list.hpp"

#include <QGroupBox>
#include <QGridLayout>
#include <QLabel>
#include <QToolBar>
#include <QStackedWidget>
#include <QAction>
#include <QMouseEvent>
#include <QStack>


class Browser_trace_info : public QGroupBox
{
public:
    Browser_trace_info(QWidget* parent) 
    : QGroupBox("Trace information", parent)
    {
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

        QGridLayout* infoLayout = new QGridLayout(this);

        infoLayout->addWidget(new QLabel("Start:", this), 0, 0);
        startTimeLabel = new QLabel("", this);
        infoLayout->addWidget(startTimeLabel, 0, 1);

        infoLayout->addWidget(new QLabel("End:", this), 1, 0);
        endTimeLabel = new QLabel("", this);
        infoLayout->addWidget(endTimeLabel, 1, 1);

        infoLayout->addWidget(new QLabel("Components:", this), 2, 0);
        componentsLabel = new QLabel("", this);
        infoLayout->addWidget(componentsLabel, 2, 1);

        infoLayout->addWidget(new QLabel("Events:", this), 3, 0);
        eventsLabel = new QLabel("", this);
        infoLayout->addWidget(eventsLabel, 3, 1);

#if 0
        QToolButton* copy = new QToolButton(this);
        copy->setIconSize(QSize(32, 32));
        copy->setToolTip("Copy trace info to clipboard");
        copy->setIcon(QIcon(":/editcopy.png"));
        infoLayout->addWidget(copy, 4, 0);
#endif

        infoLayout->setRowStretch(4, 1);
    }

    void update(Canvas* canvas)
    {
        Trace_model* model = canvas->model();

        startTimeLabel->setText(QString("%1").arg(
                                    model->min_time()));
        endTimeLabel->setText(QString("%1").arg(
                                  model->max_time()));

        QString components = QString::number(
                model->component_names().size());
   
        int hidden_components = model->unfiltered()->component_names().size()
                - model->component_names().size();

        if (hidden_components)
        {
            components += " (" 
                + QString::number(hidden_components) 
                + " more hidden)";
        }

        componentsLabel->setText(components);
        eventsLabel->setText("777");
    }



private:
    QLabel* startTimeLabel;
    QLabel* endTimeLabel;
    QLabel* componentsLabel;
    QLabel* eventsLabel;

};


class Browser_event_info : public QGroupBox
{
    Q_OBJECT
public:
    Browser_event_info(QWidget* parent) 
    : QGroupBox("Event information", parent), trace_(0)
    {
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

        QVBoxLayout* mainLayout = new QVBoxLayout(this);

        eventList = new Event_list(this);
        mainLayout->addWidget(eventList);

        QGridLayout* infoLayout = new QGridLayout(0);
        mainLayout->addLayout(infoLayout);

        infoLayout->addWidget(new QLabel("Type:", this), 0, 0);
        nameLabel = new QLabel("", this);
        infoLayout->addWidget(nameLabel, 0, 1);


        infoLayout->addWidget(new QLabel("Time:", this), 1, 0);
        timeLabel = new QLabel("", this);
        infoLayout->addWidget(timeLabel, 1, 1);

        infoLayout->setRowStretch(2, 1);

        connect(eventList, SIGNAL(currentEventChanged(Event_model*)),
                this, SLOT(currentEventChanged(Event_model*)));

    }

    void showOne(Event_model* event)
    {
        eventList->hide();
        currentEventChanged(event);
    }

    void update(Trace_model* trace, int component, 
                int min_time, int max_time)
    {
        trace_ = trace;

        QVector<bool> allow(trace->component_names().size());
        allow[component] = true;

        filtered_ = trace->set_range(min_time, max_time);
        filtered_ = filtered_->filter_components(allow);

        filtered_->rewind();

        eventList->setModel(filtered_);

        eventList->setCurrentRow(0);

        if (eventList->count() == 1)
            eventList->hide();
        else {
            eventList->show();
            eventList->setFocus(Qt::OtherFocusReason);
        }

    }

private slots:

    void currentEventChanged(Event_model* event)
    {
        nameLabel->setText(event->kind);
        
        timeLabel->setText(QString("%1").arg(event->time));
    }

private:
    QLabel* nameLabel;
    QLabel* timeLabel;
    Event_list* eventList;

    Trace_model* trace_;
    Trace_model* filtered_;
};



class Browser_state_info : public QGroupBox
{
public:
    Browser_state_info(QWidget* parent) 
    : QGroupBox("State information", parent)
    {
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

        QGridLayout* infoLayout = new QGridLayout(this);

        infoLayout->addWidget(new QLabel("Name:", this), 0, 0);
        nameLabel = new QLabel("", this);
        infoLayout->addWidget(nameLabel, 0, 1);


        infoLayout->addWidget(new QLabel("Start:", this), 1, 0);
        startTimeLabel = new QLabel("", this);
        infoLayout->addWidget(startTimeLabel, 1, 1);

        infoLayout->addWidget(new QLabel("End:", this), 2, 0);
        endTimeLabel = new QLabel("", this);
        infoLayout->addWidget(endTimeLabel, 2, 1);

        infoLayout->addWidget(new QLabel("Percentage:", this), 3, 0);
        percentageLabel = new QLabel("", this);
        infoLayout->addWidget(percentageLabel, 3, 1);



        infoLayout->setRowStretch(4, 1);
    }

    void update(State_model* state)
    {
        startTimeLabel->setText(QString("%1").arg(
                                    state->begin));
        endTimeLabel->setText(QString("%1").arg(
                                  state->end));
        nameLabel->setText(state->name);

        percentageLabel->setText("17%");
    }


private:
    QLabel* nameLabel;
    QLabel* startTimeLabel;
    QLabel* endTimeLabel;
    QLabel* percentageLabel;
};


Browser::Browser(QWidget* parent, Canvas* c)
: Tool(parent, c)
{}


class BrowserReal : public Browser
{
    Q_OBJECT
public:
    BrowserReal(QWidget* parent, Canvas* c)
    : Browser(parent, c), active_(false)
    {
        setWindowTitle("Browse trace");

        setWhatsThis("<b>Trace browser</b>"
                     "<p>The trace browser is used to navigate the trace "
                     "and get basic information about events and states.");

        zoom_in = new QPixmap(":/viewmag+.png", 0);
        if (!zoom_in->isNull())
            zoom_in_cursor = new QCursor(*zoom_in);
        zoom_out = new QPixmap(":/viewmag+.png", 0);


        QVBoxLayout* mainLayout = new QVBoxLayout(this);

        QGroupBox* help_group = new QGroupBox("Help", this);
        mainLayout->addWidget(help_group);

        QVBoxLayout* helpLayout = new QVBoxLayout(help_group);
        helpLayout->setMargin(0);

        stock_help = 
            "Object details: <b>Click</b><br>"
            "Zoom in: <b>Ctrl+Click</b><br>"
            "Zoom out: <b>Ctrl+Right Click</b>";

        helpLabel = new QLabel(stock_help, help_group);      

        helpLayout->addWidget(helpLabel);

//        QFrame* sep = new QFrame(this);
//        sep->setFrameShape(QFrame::HLine);
//        mainLayout->addWidget(sep);

        infoStack = new QStackedWidget(this);
        mainLayout->addWidget(infoStack);

        trace_info_ = new Browser_trace_info(infoStack);
        infoStack->addWidget(trace_info_);

        event_info_ = new Browser_event_info(infoStack);
        infoStack->addWidget(event_info_);

        state_info_ = new Browser_state_info(infoStack);
        infoStack->addWidget(state_info_);


#if 0
        QGroupBox* group = new QGroupBox("Event information", this);
        group->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        mainLayout->addWidget(group);

        QVBoxLayout* infoLayout = new QVBoxLayout(group);

        infoLayout->addWidget(new QLabel("Start:", group), 0, 0);
        
        startTimeLabel = new QLabel("", group);
        infoLayout->addWidget(startTimeLabel, 0, 1);

        infoLayout->addWidget(new QLabel("End:", group), 1, 0);

        endTimeLabel = new QLabel("", group);
        infoLayout->addWidget(endTimeLabel, 1, 1);

        infoLayout->addStretch();
#endif


//        mainLayout->addStretch();        

        connect(canvas(), SIGNAL(mouseEvent(QMouseEvent*, Canvas::clickTarget, 
                                            int, 
                                            State_model*, int, bool)),
                this, SLOT(canvasMouseEvent(QMouseEvent*, Canvas::clickTarget, 
                                            int,
                                            State_model*, int, bool)));

        connect(canvas(), SIGNAL(mouseMoveEvent(QMouseEvent*, 
                                                Canvas::clickTarget,
                                                int, int)),
                this, SLOT(mouseMoveEvent(QMouseEvent*,  
                                          Canvas::clickTarget,
                                          int, int)));

        connect(canvas(), SIGNAL(modelChanged(Trace_model*)),
                this, SLOT(modelChanged(Trace_model*)));
    }

    void addToolbarActions(QToolBar* toolbar)
    {
        toolbar->addSeparator();

        QAction* start = new QAction(QIcon(":/start.png"), "Start", this);
        start->setShortcut(Qt::Key_Home);
        start->setShortcutContext(Qt::ApplicationShortcut);
        connect(start, SIGNAL(triggered(bool)),
                this, SLOT(goHome()));
        toolbar->addAction(start);
        
        QAction* prev = new QAction(QIcon(":/previous.png"), "Previous", this);
        prev->setShortcut(Qt::Key_Left);
        prev->setShortcutContext(Qt::ApplicationShortcut);
        connect(prev, SIGNAL(triggered(bool)),
                this, SLOT(goLeft()));

        toolbar->addAction(prev);


        QAction* fit = new QAction(QIcon(":/fit.png"), "Fit", this);
        connect(fit, SIGNAL(triggered(bool)),
                this, SLOT(fit()));
        toolbar->addAction(fit); 

        QAction* next = new QAction(QIcon(":/next.png"), "Next", this);
        next->setShortcut(Qt::Key_Right);
        prev->setShortcutContext(Qt::ApplicationShortcut);
        connect(next, SIGNAL(triggered(bool)),
                 this, SLOT(goRight()));
        toolbar->addAction(next);

        QAction* finish = new QAction(QIcon(":/finish.png"), "End", this);
        finish->setShortcut(Qt::Key_End);
        finish->setShortcutContext(Qt::ApplicationShortcut);
        connect(finish, SIGNAL(triggered(bool)),
                this, SLOT(goEnd()));
        
        toolbar->addAction(finish);
    }


    QAction* createAction()
    {
        QAction* a = new QAction(QIcon(":/point.png"), "&Browse", this);
        a->setObjectName("browse_action");
        a->setShortcut(QKeySequence(Qt::Key_Space));
        return a;
    }

    void activate()
    {
        trace_info_->update(canvas());
        active_ = true;
    }

    void deactivate()
    {
        active_ = false;
    }

    // 'do' is to avoid naming conflict with the signal.
    void doShowEvent(Event_model* event)
    {
        infoStack->setCurrentWidget(event_info_);
        event_info_->showOne(event);
    }

private slots:

    void canvasMouseEvent(QMouseEvent* event,
                          Canvas::clickTarget target,
                          int component,
                          State_model* state,
                          int time,
                          bool events_near)
    {
        if (target == Canvas::rootClicked)
        {
            Trace_model* parent;
            if (parents.empty())
                parent = model()->enter_component(-1);
            else {
                parent = parents.top();
                parents.pop();
            }
            canvas()->setModel(parent);
        }
        else if (target == Canvas::componentClicked)
        {
            // FIXME: memory leak?
            parents.push(model());
            Trace_model* new_model = model()->enter_component(component);
            canvas()->setModel(new_model);                                
        }
        else 
        {
            if (event->modifiers() & Qt::ControlModifier)
            {
                if (event->button() == Qt::LeftButton)
                {
                    unsigned new_min = (model()->min_time() + time)/2;
                    unsigned new_max = (model()->max_time() + time)/2;
                    canvas()->setModel(
                        model()->set_range(new_min, new_max));
                }
                else if (event->button() == Qt::RightButton)
                {                                
                    int new_min = time 
                        - (time - model()->min_time())*2;
                    int new_max = time 
                        + (model()->max_time() - time)*2;

                    Trace_model* r = model()->root();
                    if ((new_max - new_min) > 
                        int(r->max_time() - r->min_time()))
                    {
                        new_min = r->min_time();
                        new_max = r->max_time();
                    }
                    else if (new_min < 0)
                    {
                        new_min = 0;
                        new_max -= new_min;
                    }
                    if (new_max > (int)r->max_time())
                    {
                        int delta = new_max - new_min;
                        new_max = r->max_time();
                        new_min = new_max - delta;
                    }

                    canvas()->setModel(
                        model()->set_range(new_min, new_max));
                }
            }
            else if (state && active_)
            {
                state_info_->update(state);
                infoStack->setCurrentWidget(state_info_);        
            }
            else if (events_near && active_)
            {
                QPair<int, int> nearby = canvas()->nearby_range(time);
                event_info_->update(model(), component, 
                                    nearby.first, nearby.second);
                infoStack->setCurrentWidget(event_info_);
            }
            else
            {            
                infoStack->setCurrentWidget(trace_info_);
            }
        }
    }

    void mouseMoveEvent(QMouseEvent* ev, Canvas::clickTarget target,
                        int component, int time)
    {
        if (target == Canvas::componentClicked
            || target == Canvas::rootClicked)
        {
            canvas()->setCursor(Qt::PointingHandCursor);
        }
        else if (target == Canvas::stateClicked && active_)
        {
            canvas()->setCursor(Qt::PointingHandCursor);
        }
        else
        {
            canvas()->setCursor(Qt::ArrowCursor);
        }
    }

    void goHome()
    {
        Trace_model* m = model();
        int delta = m->max_time() - m->min_time();

        int new_min = m->root()->min_time();

        canvas()->setModel(model()->set_range(new_min, new_min + delta));
    }

    void goEnd()
    {
        Trace_model* m = model();
        int delta = m->max_time() - m->min_time();

        int new_max = m->root()->max_time();

        canvas()->setModel(m->set_range(new_max - delta, new_max));
    }

    void fit()
    {
        Trace_model* root = model()->root();

        canvas()->setModel(
            model()->set_range(root->min_time(), root->max_time()));
    }

    void goLeft()
    {
        Trace_model* m = model();
        int delta = m->max_time() - m->min_time();

        int new_min = m->min_time()-delta/4;
        if (new_min < 0)
            new_min = 0;
                         
        int new_max = new_min + delta;

        canvas()->setModel(
            model()->set_range(new_min, new_max));
    }

    void goRight()
    {
        Trace_model* m = model();
        int delta = m->max_time() - m->min_time();

        int new_max = m->max_time()+delta/4;
        if (new_max > (int)m->root()->max_time())
        {
            new_max = m->root()->max_time();
        }
                         
        int new_min = new_max - delta;

        canvas()->setModel(
            model()->set_range(new_min, new_max));        
    }

    // This is extra hacky and is desired to work only for specific
    // case of switch from search widget to browse widget.
    void extraHelp(const QString& s)
    {
        if (s.isEmpty())
        {
            helpLabel->setText(stock_help);
        }
        else
        {
            helpLabel->setText(s + "<br>" + stock_help);
        }
    }

    void modelChanged(Trace_model*)
    {
        trace_info_->update(canvas());
    }



private:
    QString stock_help;

    QStackedWidget* infoStack;
    QLabel* helpLabel;
    Browser_trace_info* trace_info_;
    Browser_event_info* event_info_;
    Browser_state_info* state_info_;


    QPixmap* zoom_in;
    QCursor* zoom_in_cursor;
    QPixmap* zoom_out;

    QStack<Trace_model*> parents;

    bool active_;
};

Browser* createBrowser(QWidget* parent, Canvas* canvas)
{
    return new BrowserReal(parent, canvas);
}

#include "browser.moc"
