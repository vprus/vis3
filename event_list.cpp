
#include "event_list.hpp"

#include "trace_model.hpp"
#include "event_model.hpp"


Event_list::Event_list(QWidget* parent) : QListWidget(parent) 
{
    connect(this, SIGNAL(currentRowChanged(int)),
            this, SLOT(eventListRowChanged(int)));
}

void Event_list::setModel(Trace_model* model)
{
    clear();
    events.clear();

    for(;;)
    {
        std::auto_ptr<Event_model> e = model->next_event();
            
        if (!e.get())
            break;

        addItem(QString("%1 at %2").arg(e->kind).arg(e->time));
        events.push_back(e.release());
    }        
}

void Event_list::eventListRowChanged(int row)
{
    if (row != -1)
    {
        Q_ASSERT(row < events.size());

        emit currentEventChanged(events[row]);
    }
}
