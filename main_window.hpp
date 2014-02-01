
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

/** Основное окно приложения. 
    Содержит панель инструментов, изображение временной диаграммы, и набор 
    пользовательских инструментов. Инструменты деляться на "управляемые" и 
    независимые. 

    Управляемые элементы показываются в правом sidebar, и в каждый
    момент показывается только один инструмент. На панели инструментов есть набор
    кнопок, переключающих между управляемыми инструментами.

    Независимые инструмент -- это кнопка в отдельной области панели инструментов. Обработка
    нажатия на это кнопку задается автором инструмента. Например, может появляться дополнительное
    окно с графиками.
*/
class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    /** Создает новое окно, показывающее переданную трассу. 
    Набор инструментов определяется функций initialize. */
    MainWindow(Trace_model* model);

protected:
    /** Добавляет controllingWindget к списку инструментов, доступных пользователю
    в sidebar. */
    void installTool(Tool* tool);
    
    /** Добавляет инструмент навигации к списку инструментов, доступных пользователю
    в sidebar. 
    @sa Browser
    */
    void installBrowser(Browser* browser);
    
    /** Добавляет QAction для независимого инструмента к панели инструментов. Поведение
    при активации QAction определяется вызывающей стороной. */
    void installFreestandingTool(QAction*);

private:
    
    /** Данная функция создает набор используемых инструментов с
    помощью вызовов функций installTool, installBrowser и 
    installFreestandingTool. Данная реализация создает набор по умолчанию, 
    и может быть переопределена унаследованным классом если набор инструментов
    нужно изменить. */
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

