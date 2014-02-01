
#include "tool.hpp"
#include "canvas.hpp"
#include "trace_model.hpp"

#include <QGroupBox>
#include <QGridLayout>
#include <QLabel>



Trace_model* Tool::model() const 
{ 
    return canvas_->model(); 
}

QAction* Tool::action()
{
    if (!action_)
        action_ = createAction();
    return action_;
}





