
#ifndef CANVAS_HPP_VP_2006_03_20
#define CANVAS_HPP_VP_2006_03_20

#include <QScrollArea>
#include <QPair>
class QKeyEvent;

class Trace_model;
class State_model;

/** Класс для показа и взаимодействия с трассой. 

    Показывает связанный с собой экземпляр Trace_model в виде временной 
    диаграммы, и предоставляет интерфейсы, позволяющие инструментам получать уведомления о
    действиях пользователя и добавлять дополнительные графические объекты.
*/
class Canvas : public QScrollArea
{
    Q_OBJECT
public:
    Canvas(QWidget* parent);

    /** Устанавливает модель, показываемую в данный момент, и обновляет
       объект. При всех изменениях модели, этот метод должен быть вызван явно.
       Генерирует сигнал modelChanged.
    */
    void setModel(Trace_model* model);
    Trace_model* model() const;

    
    void setCursor(const QCursor& c);

    /** По вертильной координате, возвращает номер ближайшей линии жизни. */
    int nearest_lifeline(int y);

    /** По номеру компонента в времени, возвращает координаты точки. */
    QPoint lifeline_point(int component, int time);

    QPair<int, int> nearby_range(int time);

    QRect boundingRect(int component, int min_time, int max_time);

    /**
     * Тип объекта, находящегося под курсором
     * nothingClicked -- ничего интересного
     * rootClicked -- имя "корневого" компонента
     * componentClicked -- имя "обычного" компонента
     * stateClicked -- состояние
     * lifelinesClicked -- область линий жизни (исключая область
     *    где рисуются имена компонетов). Положение курсора в этом
     *    случае соответсвуюет показанному моменту времени.
     */
    enum clickTarget { nothingClicked =1, rootClicked, componentClicked,
                       stateClicked, lifelinesClicked };

    /** Добавляет новый графический элемент. Все элементы рисуются поверх собственно
        трасы. Все управление элементами должно делаться интструментами, которые их
        добавили. */
    void addItem(class CanvasItem* item);

signals:
    /** Сигнал, генерируемый при изменении модели методом setModel. */
    void modelChanged(Trace_model* new_model);

    /**
     * Сигнал, генерируемый при нажатии на какую-либо кнопку мышки.
     * @param event  Исходное событие Qt
     * @param target Тип объекта, находящегося под курсором
     * @param component Номер ближайшего с курсору компонента
     * @param state Указатель на состояние под курсором (если есть)
     * @param time Время, которому соответсвует положение курсора
     * @param events_near Флаг, говорящий есть ли поблизости к курсору события
     */
    void mouseEvent(QMouseEvent* event,
                    Canvas::clickTarget target,
                    int component,
                    State_model* state,
                    int time,
                    bool events_near);

    /**
     * Сигнал, генерируемый при движении мышкой.
     * @param event  Исходное событие Qt
     * @param target Тип объекта, находящегося под курсором
     * @param component Номер ближайшего с курсору компонента
     * @param time Время, которому соответсвует положение курсора
     */
    void mouseMoveEvent(QMouseEvent* event, Canvas::clickTarget target, 
                        int component, int time); 

protected:
    void resizeEvent(QResizeEvent* event);


private:
    void scrollContentsBy(int dx, int dy);

    friend class Contents_widget;
    class Contents_widget* contents_;
    class Timeline* timeline_;

    
};


#endif
