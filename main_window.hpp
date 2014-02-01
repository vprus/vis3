
#ifndef MAIN_WINDOW_HPP_VP_20060719
#define MAIN_WINDOW_HPP_VP_20060719

class QAction;
class QWidget;
class QMoveEvent;
class QResizeEvent;
class QActionGroup;
class QToolBar;
class QDocWidget;
class QStackedWidget;
#include <QMap>
#include <QMainWindow>

class Trace_model;
class Event_model;
class Tool;
class Canvas;
class Browser;

/** �������� ���� ����������. 
    �������� ������ ������������, ����������� ��������� ���������, � ����� 
    ���������������� ������������. ����������� �������� �� "�����������" � 
    �����������. 

    ����������� �������� ������������ � ������ sidebar, � � ������
    ������ ������������ ������ ���� ����������. �� ������ ������������ ���� �����
    ������, ������������� ����� ������������ �������������.

    ����������� ���������� -- ��� ������ � ��������� ������� ������ ������������. ���������
    ������� �� ��� ������ �������� ������� �����������. ��������, ����� ���������� ��������������
    ���� � ���������.
*/
class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    /** ������� ����� ����, ������������ ���������� ������. 
    ����� ������������ ������������ ������� initialize. */
    MainWindow(Trace_model* model);

protected:
    /** ��������� controllingWindget � ������ ������������, ��������� ������������
    � sidebar. */
    void installTool(Tool* tool);
    
    /** ��������� ���������� ��������� � ������ ������������, ��������� ������������
    � sidebar. 
    @sa Browser
    */
    void installBrowser(Browser* browser);
    
    /** ��������� QAction ��� ������������ ����������� � ������ ������������. ���������
    ��� ��������� QAction ������������ ���������� ��������. */
    void installFreestandingTool(QAction*);

private:
    
    /** ������ ������� ������� ����� ������������ ������������ �
    ������� ������� ������� installTool, installBrowser � 
    installFreestandingTool. ������ ���������� ������� ����� �� ���������, 
    � ����� ���� �������������� �������������� ������� ���� ����� ������������
    ����� ��������. */
    virtual void initialize();

    void activateAction(QAction* action);
    void addShortcuts(QWidget* w);
    void moveEvent(QMoveEvent*);
    void resizeEvent(QResizeEvent*);

private slots:
    void actionTriggered();

    void modelChanged(Trace_model* m);

    void showEvent(Event_model* event);

    void browse();

private:

    Canvas* canvas;
    Tool* currentControllingWidget;
    QActionGroup* modeActions;
    QAction* browse_action;
    Browser* browser;
    QToolBar* toolbar;
    QDockWidget* sidebar;
    QStackedWidget* sidebarContents;

    QMap<QAction*, Tool*> tools;
};


#endif

