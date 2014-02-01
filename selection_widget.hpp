
#ifndef SELECTION_WIDGET_HPP_VP_2006_04_03
#define SELECTION_WIDGET_HPP_VP_2006_04_03

#include <QGroupBox>
#include <QVector>


class Trace_model;
class QListWidget;
class QPushButton;

/** ����������� ������ ������ ������ ����� ������� ��� ���������. 

   ����� ��������� ��� ����� ��������� Trace_model:
   - ������� -- ������ ������������ � ������ ������.
   - ������� -- ������, ����������� ����������� ������ ���, ������� ������������
     ����� �������.
   - ������ -- ������, � ������� ������������ �����-���� �������.
   
   ������ �������� ������� ������������ ���� ������� ���� ���������. 
   
   ������ ���������� � ���� ������ ��� ���� ������� ��������� � ������ �������. �� ����,
   ������� ���������� � ������� ������� ��������� � ������������� �� ��������. ���������
   ���� ��� �������� ������� ������������ ��� ���������� ��� ��� � ����������� �� ����, 
   ����������� �� ��� � ������� �������. ������������ ����� ������������� ��� ����.
   
   ��� ������ ��������� ���������� ������ selectionChanged, ��� ���� ������� ������ �� 
   ��������.
*/
class SelectionWidget : public QGroupBox
{
    Q_OBJECT
public:
    /** ����������� Qt �����������. ����� ������ ������ ����� ������, � ������
       ���� ��������������� ������� ������ compoments_setModel ��� 
       events_setModel. */
    SelectionWidget(const QString& name, QWidget* parent);

    /** ������������� ������ ����� ������ ���������. ������� � ������� Trace_model 
       ���������� ��� ���������, ������ Trace_model ���������� �������
       base->unfiltered.
       ����� ����� ���������� ������������ ����� ���, ������ ����� �������� � ������
       �����������������. */
    void components_setModel(Trace_model* base, Trace_model* current);

    /** ��������� ���������� components_setModel, �� ���������� ���� �������. */
    void events_setModel(Trace_model* base, Trace_model* current);

    /** ���������� ������, ����������� ����� ���� �������. ������ ������� ����� ����������
       ����� �������/���������/��������� � ������� ������� Trace_model. */
    const QVector<bool>& selection() const;

signals:
    /** ������ ������������ ��� ��������� ������. */
    void selectionChanged(const QVector<bool>&);

private slots:

    void showOrHideAll();

    void eventListChanged();

private:
    
    /** ������������� ������������ ������ ����� �� ���� �������
       -- ��� �������, �������� � �������� ������� Trace_model.
       ������� ������ ��� �����, �������� �� �� � ��������� ���
       �����������.
    */
    void SelectionWidget::initializeFromTypes(const std::vector<QString>& full,
                                              const std::vector<QString>& base,
                                              const std::vector<QString>& current);
    
    QListWidget* eventList;
    QPushButton* showAll;

    /* For each event/state type inside the base Tree_model, tells if it's selected. */
    QVector<bool> selection_;
    /* Maps from index in eventList to index in selection_. Value is -1
    is there's no corresponding index. */
    QVector<int> item2type_;
    
    
};

#endif
