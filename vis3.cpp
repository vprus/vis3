
#include "tools/tool.hpp"

#include "canvas.hpp"
#include "trace_model.hpp"
#include "state_model.hpp"
#include "event_model.hpp"
#include "canvas_item.hpp"
#include "group_model.hpp"

#include "event_list.hpp"

#include "main_window.hpp"

#include <qwidget.h>
#include <qapplication.h>
#include <qtoolbar.h>
#include <qmainwindow.h>
#include <qicon.h>
#include <QVBoxLayout>
#include <qlabel.h>
#include <qdockwidget.h>
#include <qpainter.h>
#include <QFrame>
#include <QAction>
#include <QFile>
#include <QDomDocument>
#include <QDomElement>
#include <QVector>
#include <QStackedWidget>
#include <QGroupBox>
#include <QGridLayout>
#include <QRadioButton>
#include <QSpinBox>
#include <QSettings>
#include <QPushButton>
#include <QKeyEvent>
#include <QTableWidget>
#include <QListWidget>
#include <QTimer>
#include <QToolButton>
#include <QTextEdit>
#include <QShortcut>
#include <QWhatsThis>

#include <sstream>
#include <iostream>
#include <set>
#include <algorithm>




#if 0
class Fake_trace_model : public Trace_model
{
public:
    unsigned num_components() const { return 10; }

    std::string parent_name() const { return "Model"; }

    std::string component_name(int i) const 
    {
        std::stringstream ss;
        ss << "Component " << i;
        return ss.str();
    }

    bool has_children(int i) const { return i % 2; }

    unsigned min_time() const { return 40; }

    unsigned max_time() const { return 180; }

    void rewind()
    {
        current_state = 0;
    }
    
    std::auto_ptr<State_model> next_state()
    {
        if (current_state == 3)
            return std::auto_ptr<State_model>(0);

        std::auto_ptr<State_model> r(new State_model);

        r->begin = 40 + 10 + 20*current_state;
        r->end = r->begin + 30;
        r->component = 5 + current_state;
        r->name = QString("State %1").arg(current_state).toLatin1();

        ++current_state;

        return r;
    }


    std::auto_ptr<Event_model> next_event()
    {
        return std::auto_ptr<Event_model>();
    }

    Trace_model* root()
    {
        return 0;
    }

    Trace_model* enter_component(int)
    {
        return this;
    }


    unsigned current_state;
};

#endif

class XML_test_model : public Trace_model
{
public:
    XML_test_model(const QString& filename)
    : min_time_(40), max_time_(180)
    {
        QDomDocument doc("trace");

        QFile file(filename);
        if (!file.open(QIODevice::ReadOnly))
            return;
        if (!doc.setContent(&file))
        {
            file.close();
            return;
        }
        file.close();

        root_ = doc.documentElement();

        currentElement = doc.documentElement().firstChildElement();
        allowed_event_kinds = all_event_kinds();

        initialize();        
    }

    std::string parent_name() const 
    { 
        if (currentElement.tagName() == "component")
            return currentElement.attribute("name").toLatin1().data();
        else
            return "";
    }

    const std::vector<QString>& component_names() const
    {
        return componentNames_;
    }

    bool has_children(int i) const 
    {
        return !subcomponents.at(i).toElement().
            elementsByTagName("component").isEmpty();
    }

    unsigned min_time() const { return min_time_; }

    unsigned max_time() const { return max_time_; }

    void rewind()
    {
        currentItem.clear();
        currentSubcomponent = -1;

        allEvents.clear();
        QMultiMap<int, Event_model*> events;

        for(;;)
        {
            std::auto_ptr<Event_model> e = next_event_unsorted();

            Event_model* ep = e.release();

            if (!ep)
                break;

            events.insert(ep->time, ep);
        }
        
        for(QMultiMap<int, Event_model*>::iterator i = events.begin(),
                e = events.end(); i != e; ++i)
        {
            allEvents.push_back(i.value());
        }
        currentEvent = 0;

        currentItem.clear();
        currentSubcomponent = -1;
    }

    std::auto_ptr<State_model> next_state()
    {
        findNextItem("states");

        if (currentItem.isNull())
            return std::auto_ptr<State_model>();
        else
        {
            std::auto_ptr<State_model> r(new State_model);
            
            r->begin = currentItem.attribute("begin").toInt();
            r->end = currentItem.attribute("end").toInt();
            r->component = currentSubcomponent;
            r->name = currentItem.attribute("name").toLatin1();

            return r;
        }
    }

    std::auto_ptr<Group_model> next_group()
    {
        findNextItem("groups");

        if (currentItem.isNull())
        {
            return std::auto_ptr<Group_model>();
        }
        else
        {
            std::auto_ptr<Group_model> r(new Group_model);

            r->from_component = currentSubcomponent;
            r->from_time = currentItem.attribute("time").toInt();
            r->to_time = currentItem.attribute("target_time").toInt();

            QString to_component_s = currentItem.attribute("target_component");

            // Hack, hack. We should not return groups where target compoment
            // is hidden.
            r->to_component = -1;
            for(unsigned i = 0; i < subcomponents.size(); ++i)
            {
                if (to_component_s == subcomponents[i].attribute("name"))
                {
                    r->to_component = i;
                    break;
                }
            }

            return r;
        }
    }

    std::auto_ptr<Event_model> next_event_unsorted()
    {
        unsigned time = 0;
        for(;;)
        {
            findNextItem("events");

            if (currentItem.isNull())
                break;

            time = currentItem.attribute("time").toInt();

            QString kind_att = currentItem.attribute("kind");
            bool allowed_kind = false;
            if (kind_att.isEmpty())
            {
                allowed_kind = true;
            }
            else
            {
                for(unsigned i = 0; i < allowed_event_kinds.size(); ++i)
                {
                    if (allowed_event_kinds[i] == kind_att)
                    {
                        allowed_kind = true;
                        break;
                    }
                }
            }

            if (time >= min_time_ && time <= max_time_ && allowed_kind)
                break;                
        }

        if (currentItem.isNull())
            return std::auto_ptr<Event_model>();
        else
        {
            std::auto_ptr<Event_model> r(new Event_model);
            
            r->time = currentItem.attribute("time").toInt();
            r->letter = currentItem.attribute("letter")[0].toLatin1();
            r->kind = currentItem.attribute("kind").toLatin1();
            r->component = currentSubcomponent;

            return r;
        }   
    }


    std::auto_ptr<Event_model> next_event()
    {
        if (currentEvent >= allEvents.count())
        {
            return std::auto_ptr<Event_model>();
        }
        else
        {
            return std::auto_ptr<Event_model>(allEvents[currentEvent++]);
        }

#if 0
        if (currentItem.isNull())
            return std::auto_ptr<Event_model>();
        else
        {
            std::auto_ptr<Event_model> r(new Event_model);
            
            r->time = currentItem.attribute("time").toInt();
            r->letter = currentItem.attribute("letter")[0].toAscii();
            r->component = currentSubcomponent;

            return r;
        }   
#endif
    }

    Trace_model* root()
    {
        XML_test_model* n = new XML_test_model(*this);
        // Seems like document can be already destroyed, so 
        // using currentElement.ownerDocument() won't work.
        n->currentElement = root_;
        n->min_time_ = 0;
        QString max_time_at = n->currentElement.attribute("max_time");
        if (max_time_at.isEmpty())
        {
            n->max_time_ = max_time_;
        }
        else
        {
            n->max_time_ = max_time_at.toInt();
        }
        n->allowed_event_kinds = all_event_kinds();
        n->initialize();
        return n;        
    }

    Trace_model* enter_component(int i)
    {
        if (i == -1)
        {
            if (currentElement.tagName() != "trace")
            {
                XML_test_model* n = new XML_test_model(*this);
                n->currentElement = currentElement.parentNode().toElement();
                n->initialize();
                return n;
            }
            else
            {
                return this;
            }
        }
        else
        {
            XML_test_model* n = new XML_test_model(*this);
            n->currentElement = subcomponents[i];
            n->initialize();
            return n;
        }
        return this;
    }

    Trace_model* set_range(unsigned min, unsigned max)
    {
        XML_test_model* n = new XML_test_model(*this);        
        n->min_time_ = min;
        n->max_time_ = max;
        return n;
    }

    Trace_model* filter_components(const QVector<bool>& allow)
    {
        XML_test_model* n = new XML_test_model(*this);        
        n->subcomponents.clear();
        n->componentNames_.clear();

        for(unsigned i = 0; i < subcomponents.size(); ++i)
        {
            if (allow[i])
            {
                n->subcomponents.push_back(subcomponents[i]);
                n->componentNames_.push_back(componentNames_[i]);
            }
        }

        return n;        
    }


    Trace_model* unfiltered()
    {
        XML_test_model* n = new XML_test_model(*this);
        n->initialize();
        n->allowed_event_kinds = all_event_kinds();
        return n;
    }

    std::vector<QString> event_kinds()
    {        
        return allowed_event_kinds;
    }

    std::vector<QString> all_event_kinds()
    {
        std::vector<QString> v;
        bool initialized = false;
        if (!initialized)
        {
            initialized = true;
            v.push_back("Update");
             v.push_back("Delay");
            v.push_back("Send");
            v.push_back("Receive");
            v.push_back("Stop");
        }

        return v;
    }

    Trace_model* filter_events(const QVector<bool>& allow)
    {
        XML_test_model* n = new XML_test_model(*this);

        n->allowed_event_kinds.clear();
        
        for(unsigned i = 0; i < allowed_event_kinds.size(); ++i)
        {
            if (allow[i])
            {
                n->allowed_event_kinds.push_back(allowed_event_kinds[i]);
            }
        }
        

        return n;
    }

    QString save() const
    {
        QString componentPos = component_tree_location();

        QStringList event_kinds;
        for(unsigned i = 0; i < allowed_event_kinds.size(); ++i)
        {
            event_kinds += allowed_event_kinds[i];
        }

        QStringList components;
        for(unsigned i = 0; i < subcomponents.size(); ++i)
        {
            components += subcomponents[i].attribute("name");
        }
        
        return  componentPos + ":" + 
            QString("%1:%2").arg(min_time_).arg(max_time_)
            + ":" + event_kinds.join(",")
            + ":" + components.join(",");
    }
    
    void restore(const QString& s)
    {
        QStringList parts = s.split(":");
        QString path = parts[0];
        
        QStringList path_parts = path.split("/");
        currentElement = root_;
        foreach(const QString& s, path_parts)
        {
            for(QDomElement e = currentElement.firstChildElement("component");
                !e.isNull();
                e = e.nextSiblingElement("component"))
            {
                if (e.attribute("name") == s)
                {
                    currentElement = e;
                    break;
                }
            }
            // FIXME: maybe, if we did not found the element, we should
            // not restore event/state filters?


        }
        min_time_ = parts[1].toInt();
        max_time_ = parts[2].toInt();

        QStringList eventList = parts[3].split(",");
        allowed_event_kinds.assign(eventList.begin(), eventList.end());

        initialize();

        QStringList componentList = parts[4].split(",");
        QSet<QString> componentSet = QSet<QString>::fromList(componentList);
        std::vector<QDomElement> new_subcomponents;
        std::vector<QString> newComponentNames;

        /* FIXME: I don't remember why we need 'new_subcomponents', and
        the conditiona inside the loop.*/
        for(unsigned i = 0; i < subcomponents.size(); ++i)
        {
            if (componentSet.contains(subcomponents[i].attribute("name")))
            {
                new_subcomponents.push_back(subcomponents[i]);
                newComponentNames.push_back(componentNames_[i]);
            }
        }
        subcomponents = new_subcomponents;
        componentNames_ = newComponentNames;
    }

    QString component_tree_location() const
    {
        QList<QString> path;

        for(QDomElement c = currentElement; 
            !c.isNull() && c.tagName() == "component";
            c = c.parentNode().toElement())
        {
            path.push_back(c.attribute("name"));
        }
        std::reverse(path.begin(), path.end());
        
        return QStringList(path).join("/");
    }

private:
    void initialize()
    {
        subcomponents.clear();
        componentNames_.clear();

        for(QDomElement e = currentElement.firstChildElement("component"); 
            !e.isNull(); 
            e = e.nextSiblingElement("component"))
        {
            subcomponents.push_back(e);
            
            componentNames_.push_back(e.toElement().attribute("name"));
        }        
    }

    void findNextItem(const QString& elementName)
    {
        if (!currentItem.isNull())
            currentItem = currentItem.nextSiblingElement();

        if (currentItem.isNull())
        {
            for(;;)
            {
                ++currentSubcomponent;
                if (unsigned(currentSubcomponent) >= subcomponents.size())
                    break;
              
                QDomElement group = subcomponents[currentSubcomponent]
                    .firstChildElement(elementName);

                if (!group.isNull())
                {
                    currentItem = group.firstChildElement();
                    break;
                }
            }
        }
    }
     
    QDomElement currentElement;

    std::vector<QDomElement> subcomponents;
    std::vector<QString> componentNames_;
    std::vector<QString> allowed_event_kinds;

    // The current item we iterate over -- could be state,
    // or event, or group event.
    QDomElement currentItem;
    int currentSubcomponent;

    // All events sorted by the time.
    QVector<Event_model*> allEvents;
    int currentEvent;

    // Need to hold a reference to root so that it's not
    // deleted.
    QDomElement root_;

    unsigned min_time_, max_time_;

};


MainWindow::MainWindow(Trace_model *model) : QMainWindow(0), currentControllingWidget(0),
                   modeActions(new QActionGroup(this))
{
    QSettings settings;
    QRect r = settings.value("main window/geometry").toRect();
    if (r.isValid())
        setGeometry(r);

    QString model_desc = settings.value("current model").toString();
    if (!model_desc.isEmpty())
    {
        model->restore(model_desc);
    }

    canvas = new Canvas(this);

    canvas->setModel(model);

    setCentralWidget(canvas);

    toolbar = new QToolBar(this);
    addToolBar(toolbar);

    toolbar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

    sidebar = new QDockWidget(this);
    addDockWidget(Qt::LeftDockWidgetArea, sidebar);
    sidebar->setWindowTitle("Browse trace");
    sidebar->setFeatures(QDockWidget::NoDockWidgetFeatures);

    sidebarContents = new QStackedWidget(sidebar);
    sidebar->setWidget(sidebarContents);

    initialize();

    toolbar->addActions(modeActions->actions());

    Q_ASSERT(browser);
    browser->addToolbarActions(toolbar);

    toolbar->addSeparator();

    toolbar->addAction(QWhatsThis::createAction(this));

    addShortcuts(toolbar);

    bool action_set = false;
    QString current_action = settings.value("current action").toString();
    if (!current_action.isEmpty())
    {
        if (QAction* a = findChild<QAction*>(current_action))
        {
            a->setChecked(true);
            activateAction(a);
            action_set = true;
        }
    }

    if (!action_set) {
        browse_action->setChecked(true);
        activateAction(browse_action);
    }

    // Ugly way to move focus to canvas.
    canvas->setModel(canvas->model());
    
    connect(canvas, SIGNAL(modelChanged(Trace_model*)),
            this, SLOT(modelChanged(Trace_model*)));
}

void MainWindow::installTool(Tool* controllingWidget)
{
    QAction* action = controllingWidget->action();

    action->setCheckable(true);

    tools[action] = controllingWidget;

    sidebarContents->addWidget(controllingWidget);        
    modeActions->addAction(action);

    connect(action, SIGNAL(triggered(bool)),
            this, SLOT(actionTriggered()));

    connect(controllingWidget, SIGNAL(showEvent(Event_model*)),
            this, SLOT(showEvent(Event_model*)));

    connect(controllingWidget, SIGNAL(browse()),
            this, SLOT(browse()));

    action->setWhatsThis(controllingWidget->whatsThis());
}

void MainWindow::initialize()
{
    Browser* browser = createBrowser(sidebarContents, canvas);
    installBrowser(browser);
    installTool(createGoto(sidebarContents, canvas));
    installTool(createMeasure(sidebarContents, canvas));
    installTool(createFilter(sidebarContents, canvas));
    Tool* find = createFind(sidebarContents, canvas);
    installTool(find);
    
    connect(find, SIGNAL(extraHelp(const QString&)),
            browser, SLOT(extraHelp(const QString&)));


}

void MainWindow::installBrowser(Browser* browser)
{
    installTool(browser);
    this->browser = browser;
    browse_action = browser->action();
}

void MainWindow::activateAction(QAction* action)
{
    if (currentControllingWidget)
        currentControllingWidget->deactivate();

    Tool* c = tools[action];
    sidebarContents->setCurrentWidget(c);
    sidebar->setWindowTitle(c->windowTitle());

    currentControllingWidget = c;
    c->activate();

    QSettings settings;
    QString actionName = action->objectName();

    if (!actionName.isEmpty())
    {
        settings.setValue("current action", actionName);
    }
}

void MainWindow::addShortcuts(QWidget* w)
{
    foreach(QAction* action, w->actions())
    {
        QString tooltip = action->toolTip();
        if (!tooltip.isEmpty())
        {
            if (!action->shortcut().isEmpty())
            {
                action->setToolTip(tooltip + " (Shortcut: <b>" 
                        + action->shortcut().toString()
                        + "</b>)");
            }
        }
    }
}

void MainWindow::moveEvent(QMoveEvent*)
{
    QSettings settings;
    settings.setValue("main window/geometry", geometry());
}

void MainWindow::resizeEvent(QResizeEvent*)
{
    QSettings settings;
    settings.setValue("main window/geometry", geometry());
}


void MainWindow::actionTriggered()
{
    activateAction(static_cast<QAction*>(sender()));
}

void MainWindow::modelChanged(Trace_model* m)
{
    QSettings settings;
    settings.setValue("current model", m->save());
}

void MainWindow::showEvent(Event_model* event)
{
    browser->doShowEvent(event);
    activateAction(browse_action);
}

void MainWindow::browse()
{
    activateAction(browse_action);
}

int main(int ac, char* av[])
{
    QApplication app(ac, av);
    app.setOrganizationName("Computer Systems Laboratory");
    app.setOrganizationDomain("lvk.cs.msu.su");
    app.setApplicationName("Vis3");


    QFont f("Arial");
    f.setPointSize(11);
    app.setFont(f);

    XML_test_model* model = new XML_test_model("trace.xml");

    MainWindow mw(model);
    mw.show();


    app.exec();
    return 0;    
}

