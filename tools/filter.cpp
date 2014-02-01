
#include "tool.hpp"

#include "canvas.hpp"
#include "trace_model.hpp"
#include "selection_widget.hpp"

#include <QVBoxLayout>
#include <QAction>

class Filter : public Tool
{
    Q_OBJECT
public:
    Filter(QWidget* parent, Canvas* c) 
    : Tool(parent, c)
    {
        setWindowTitle("Filter");

        QVBoxLayout* mainLayout = new QVBoxLayout(this);

        components = new SelectionWidget("Components", this);
        mainLayout->addWidget(components);

        components->components_setModel(model()->unfiltered(), model());

        events = new SelectionWidget("Events", this);
        mainLayout->addWidget(events);

        events->events_setModel(model()->unfiltered(), model());

        // Filtering by states not implemented for now.
#if 0
        states = new SelectionWidget("States", this);
        mainLayout->addWidget(states);
#endif

        connect(components, SIGNAL(selectionChanged(const QVector<bool>&)),
                this, SLOT(filtersChanged(const QVector<bool>&)));

        connect(events, SIGNAL(selectionChanged(const QVector<bool>&)),
                this, SLOT(filtersChanged(const QVector<bool>&)));

        connect(canvas(), SIGNAL(modelChanged(Trace_model*)),
                this, SLOT(modelChanged(Trace_model*)));

        modelChanged(model());

    }

    QAction* createAction()
    {
        QAction* filter_action = new QAction(QIcon(":/filter.png"), "Fi&lter", 
                                             this);
        filter_action->setObjectName("filter_action");
        filter_action->setShortcut(QKeySequence(Qt::Key_L));
        return filter_action;
    }

    void activate() {}
    void deactivate() {}

private slots:
    void modelChanged(Trace_model* model)
    {
        components->components_setModel(model->unfiltered(), model);
        events->events_setModel(model->unfiltered(), model);        
    }

    void filtersChanged(const QVector<bool>& new_selection)
    {
        // Apply all filters in order on any change.

        Trace_model* filtered = model()
            ->unfiltered()->filter_components(components->selection());

        filtered = filtered->filter_events(events->selection());

        canvas()->setModel(filtered);

        modelChanged(model());
    }

private:
    SelectionWidget* components;
    SelectionWidget* events;
    SelectionWidget* states;
};

Tool* createFilter(QWidget* parent, Canvas* canvas)
{
    return new Filter(parent, canvas);
}

#include "filter.moc"
