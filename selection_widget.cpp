
#include "selection_widget.hpp"

#include "trace_model.hpp"

#include <QVBoxLayout>
#include <QListWidget>
#include <QPushButton>

#include <set>
#include <iostream>


SelectionWidget::SelectionWidget(const QString& name, QWidget* parent) 
: QGroupBox(name, parent)
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
       
    eventList = new QListWidget(this);
    mainLayout->addWidget(eventList);

    QHBoxLayout* eventButtons = new QHBoxLayout();
    mainLayout->addLayout(eventButtons);
        
    eventButtons->addStretch();
    showAll = new QPushButton("Show All", this);
    eventButtons->addWidget(showAll);
    QPushButton* hideAll = new QPushButton("Hide All", this);
    eventButtons->addWidget(hideAll);                

    connect(showAll, SIGNAL(clicked(bool)),
            this, SLOT(showOrHideAll()));

    connect(hideAll, SIGNAL(clicked(bool)),
            this, SLOT(showOrHideAll()));

    connect(eventList, SIGNAL(itemChanged(QListWidgetItem*)),
            this, SLOT(eventListChanged()));

}

void SelectionWidget::components_setModel(Trace_model* base, Trace_model* current)
{
    initializeFromTypes(base->unfiltered()->component_names(),
                        base->component_names(),
                        current->component_names());
}

void SelectionWidget::events_setModel(Trace_model* base, Trace_model* current)
{
    initializeFromTypes(base->unfiltered()->event_kinds(),
                        base->event_kinds(),
                        current->event_kinds());
}

void SelectionWidget::initializeFromTypes(const std::vector<QString>& full, 
                                          const std::vector<QString>& base,
                                          const std::vector<QString>& current)
{
    eventList->blockSignals(true);
    eventList->clear();
    selection_.clear();
    
    std::cout << "full size : " << full.size() << " base size: " << base.size() << "\n";
    
    std::set<QString> in_base(base.begin(), base.end());
    std::set<QString> in_current(current.begin(), current.end());
    
    
    
    for(unsigned i = 0; i < full.size(); ++i)
    {
        QString name = full[i];
        QListWidgetItem* it = new QListWidgetItem(name, eventList);
        
        if (in_base.count(name))
        {
            item2type_.push_back(selection_.size());
            if (in_current.count(name))
            {
                it->setCheckState(Qt::Checked);
                selection_.push_back(true);
            }
            else
            {
                it->setCheckState(Qt::Unchecked);
                selection_.push_back(false);
            }
        }
        else
        {
            item2type_.push_back(-1);
            it->setCheckState(Qt::Unchecked);
            it->setFlags(0);
        }
    }

    eventList->blockSignals(false);
    
    
}

const QVector<bool>& SelectionWidget::selection() const
{
    return selection_;
}



void SelectionWidget::showOrHideAll()
{
    // Prevent eventListChanged from being emitted for each item.
    eventList->blockSignals(true);

    bool show = (sender() == showAll);

    for(int i = 0; i < eventList->count(); ++i)
    {
        if (item2type_[i] == -1)
        {
            /* This is disabled item, present in unfiltered Trace_model,
            but not present in base Trace_model. Don't change its state at all.
            */
            continue;
        }
        QListWidgetItem* it = eventList->item(i);
        if (show)
            it->setCheckState(Qt::Checked);
        else
            it->setCheckState(Qt::Unchecked);
    }

    eventList->blockSignals(false);
    eventListChanged();
}

void SelectionWidget::eventListChanged()
{
    for(int i = 0; i < eventList->count(); ++i)
    {
        int si = item2type_[i];
        if (si != -1)
        {
            selection_[si] = (eventList->item(i)->checkState() == Qt::Checked);
        }
    }

    emit selectionChanged(selection_);
}
