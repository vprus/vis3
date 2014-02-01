
#ifndef TOOL_HPP_VP_2006_04_03
#define TOOL_HPP_VP_2006_04_03

#include <QWidget>

class QAction;
class QToolBar;


class Canvas;
class Trace_model;
class Event_model;

/** ������� ����� ��� ������������, ����������� � ����� sidebar.

   � ������ ������ ������� ����� ������ ���� ����������. Toolbar
   �������� ��������� ������� � ��������, �������������� �������
   ����������.

   ����� Tool ���������� �� QWidget � ��������������� ������������ �
   ����� sidebar.
*/
class Tool : public QWidget
{
    Q_OBJECT
public: 
    Tool(QWidget* parent, Canvas* canvas) 
    : QWidget(parent), canvas_(canvas), action_(0) {}

    /** ���������� ������ QAction, ������� ���������� �� toolbar. */
    QAction* action();

    /** ������ ����� ���������� � ������ ����������� �����������,
        ����� �� ���������� ������� �� sidebar. */
    virtual void activate() = 0;

    /** ������ ����� ���������� � ������ ������������� �����������,
    ����� �� ���������� ��������� �� sidebar. */
    virtual void deactivate() = 0;

signals:
    /**
     * ������ ������ ����� �������������� ��������� �������� ��� ����,
     * ����� �������� ��������� ���������� � ������� event. ������������
     * ������������ ��� ���� ���������� �������������.
     */
    void showEvent(Event_model* event);

    /**
     *  ������ ������ ����� �������������� ��������� �������� ��� ������������
     *  �� ��������� "������ ������ ������".
     */
    void browse();
    
protected:
    /**
     *  ���������� �����, �� ������� ������ ���������� ����� ��������.
     */
    Canvas* canvas() const { return canvas_; }
    /**
     *  ���������� ������, � ������� �������� ������ ����������.
     */
    Trace_model* model() const;

private:
    /** ������� ������ ������� ������ Action, ������� ����� �������
        �� ������ ������������. ������ ������ ����� ���������  ������,
        � ��� ������� (objectName). ������� ������� ������ ��������������
        �� ������. */
    virtual QAction* createAction() = 0;

private:
    Canvas* canvas_;
    QAction* action_;
};

/** ����������, ��������������� ��� ��������� �� ��������� ��������. ���� �� ������������
 ������ ������ ���� ������������ �� ����� ����, � �������� � MainWindow ������� 
������ MainWindow::installBrowser.
��������� �������� ���� � �����, �������� �� ������� ����� � ������, ��������� ��������,
� ����� ������ ������� � ��������� ���������� �� ������.
*/
class Browser : public Tool
{
    Q_OBJECT
public:
    Browser(QWidget* parent, Canvas* c);

    /** ���������� ������� MainWindow. ������ �������� ��������, �����������
    ��� ���������, � ����� toolbar. */
    virtual void addToolbarActions(QToolBar* toolbar) = 0;
    /** ��� ������ ������ �������� ��������� ���������� � ������� event. */
    virtual void doShowEvent(Event_model* event) = 0;
};




Browser* createBrowser(QWidget* parent, Canvas* canvas);
Tool* createGoto(QWidget* parent, Canvas* canvas);
Tool* createMeasure(QWidget* parent, Canvas* canvas);
Tool* createFilter(QWidget* parent, Canvas* canvas);
Tool* createFind(QWidget* parent, Canvas* canvas);




#endif
