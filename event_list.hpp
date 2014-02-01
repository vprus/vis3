
#ifndef EVENT_LIST_HPP_VP_2006_04_03
#define EVENT_LIST_HPP_VP_2006_04_03

#include <QListWidget>
#include <QVector>

class Trace_model;
class Event_model;

class Event_list : public QListWidget
{
    Q_OBJECT
public:
    Event_list(QWidget* parent);

    void setModel(Trace_model* model);

signals:
    void currentEventChanged(Event_model*);

private slots:
    void eventListRowChanged(int row);

private:

    QVector<Event_model*> events;    
};



#endif
