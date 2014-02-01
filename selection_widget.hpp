
#ifndef SELECTION_WIDGET_HPP_VP_2006_04_03
#define SELECTION_WIDGET_HPP_VP_2006_04_03

#include <QGroupBox>
#include <QVector>


class Trace_model;
class QListWidget;
class QPushButton;

/** Стандартный диалог выбора набора типов событий или состояний. 

   Класс оперирует над тремя объектами Trace_model:
   - текущий -- объект показываемый в данный момент.
   - базовый -- объект, определящий максимально полный вид, который пользователь
     может выбрать.
   - полный -- объект, в котором отсутствуюет какие-либо фильтры.
   
   Каждый экземляр диалога обрабатывает либо события либо состояния. 
   
   Диалог показывает в виде списка все типы событий имеющиеся в полном объекте. Те типы,
   которые отсутсвуют в базовом объекте неактивны и пользователем не меняются. Остальные
   типы при создание диалога показываются как выделенные или нет в зависимости от того, 
   присутсвуют ли они в текущем объекты. Пользователь может перещелкивать эти типы.
   
   При каждом изменении вызывается сигнал selectionChanged, при этом текущий объект не 
   менятеся.
*/
class SelectionWidget : public QGroupBox
{
    Q_OBJECT
public:
    /** Стандартный Qt конструктор. После вызова список типов пустой, и должен
       быть инициализирован вызовом метода compoments_setModel или 
       events_setModel. */
    SelectionWidget(const QString& name, QWidget* parent);

    /** Инициализиует список типов типами компонент. Базовый и текущий Trace_model 
       передаются как параматры, полный Trace_model получается вызовом
       base->unfiltered.
       Метод может вызываться произвольное число раз, каждый вызов приводит к полной
       переинициализации. */
    void components_setModel(Trace_model* base, Trace_model* current);

    /** Полностью аналогичен components_setModel, но использует типы событий. */
    void events_setModel(Trace_model* base, Trace_model* current);

    /** Возвращает вектор, указывающий какие типы выбраны. Размер вектора равен количеству
       типов событий/состояний/компонент в базовом объекте Trace_model. */
    const QVector<bool>& selection() const;

signals:
    /** Сигнал генерируемый при изменении выбора. */
    void selectionChanged(const QVector<bool>&);

private slots:

    void showOrHideAll();

    void eventListChanged();

private:
    
    /** Инициализиует показываемый список типов по трем спискам
       -- для полного, базового и текущего объекта Trace_model.
       Данному методу все равно, работаем ли мы с собитиями или
       состояниями.
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
