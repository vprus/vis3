
#ifndef CANVAS_HPP_VP_2006_03_20
#define CANVAS_HPP_VP_2006_03_20

#include <QScrollArea>
#include <QPair>
class QKeyEvent;

class Trace_model;
class State_model;

/** ����� ��� ������ � �������������� � �������. 

    ���������� ��������� � ����� ��������� Trace_model � ���� ��������� 
    ���������, � ������������� ����������, ����������� ������������ �������� ����������� �
    ��������� ������������ � ��������� �������������� ����������� �������.
*/
class Canvas : public QScrollArea
{
    Q_OBJECT
public:
    Canvas(QWidget* parent);

    /** ������������� ������, ������������ � ������ ������, � ���������
       ������. ��� ���� ���������� ������, ���� ����� ������ ���� ������ ����.
       ���������� ������ modelChanged.
    */
    void setModel(Trace_model* model);
    Trace_model* model() const;

    
    void setCursor(const QCursor& c);

    /** �� ���������� ����������, ���������� ����� ��������� ����� �����. */
    int nearest_lifeline(int y);

    /** �� ������ ���������� � �������, ���������� ���������� �����. */
    QPoint lifeline_point(int component, int time);

    QPair<int, int> nearby_range(int time);

    QRect boundingRect(int component, int min_time, int max_time);

    /**
     * ��� �������, ������������ ��� ��������
     * nothingClicked -- ������ �����������
     * rootClicked -- ��� "���������" ����������
     * componentClicked -- ��� "��������" ����������
     * stateClicked -- ���������
     * lifelinesClicked -- ������� ����� ����� (�������� �������
     *    ��� �������� ����� ����������). ��������� ������� � ����
     *    ������ ������������� ����������� ������� �������.
     */
    enum clickTarget { nothingClicked =1, rootClicked, componentClicked,
                       stateClicked, lifelinesClicked };

    /** ��������� ����� ����������� �������. ��� �������� �������� ������ ����������
        �����. ��� ���������� ���������� ������ �������� ��������������, ������� ��
        ��������. */
    void addItem(class CanvasItem* item);

signals:
    /** ������, ������������ ��� ��������� ������ ������� setModel. */
    void modelChanged(Trace_model* new_model);

    /**
     * ������, ������������ ��� ������� �� �����-���� ������ �����.
     * @param event  �������� ������� Qt
     * @param target ��� �������, ������������ ��� ��������
     * @param component ����� ���������� � ������� ����������
     * @param state ��������� �� ��������� ��� �������� (���� ����)
     * @param time �����, �������� ������������ ��������� �������
     * @param events_near ����, ��������� ���� �� ���������� � ������� �������
     */
    void mouseEvent(QMouseEvent* event,
                    Canvas::clickTarget target,
                    int component,
                    State_model* state,
                    int time,
                    bool events_near);

    /**
     * ������, ������������ ��� �������� ������.
     * @param event  �������� ������� Qt
     * @param target ��� �������, ������������ ��� ��������
     * @param component ����� ���������� � ������� ����������
     * @param time �����, �������� ������������ ��������� �������
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
