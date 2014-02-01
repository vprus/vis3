
#include "tool.hpp"

#include "trace_model.hpp"
#include "canvas.hpp"

#include <QSpinBox>
#include <QRadioButton>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QSettings>
#include <QAction>
#include <QKeyEvent>

class SpinBoxWithFocusReport : public QSpinBox
{
    Q_OBJECT
public:
    SpinBoxWithFocusReport(QWidget* parent) : QSpinBox(parent) {}

protected:
    void focusInEvent(QFocusEvent* event)
    {
        QSpinBox::focusInEvent(event);
        emit focusIn();
    }

signals:
    void focusIn();
};


class Goto : public Tool
{
    Q_OBJECT
public:
    Goto(QWidget* parent, Canvas* c) : Tool(parent, c)
    {
        setWindowTitle("Goto");

        setFocusPolicy(Qt::StrongFocus);

        QVBoxLayout* mainLayout = new QVBoxLayout(this);

        QGroupBox* group = new QGroupBox("Goto", this);
        group->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        mainLayout->addWidget(group);

        QVBoxLayout* gotoLayout = new QVBoxLayout(group);

        setStart = new QRadioButton("Set start", group);
        setStart->setObjectName("setStart");
        gotoLayout->addWidget(setStart);

        QHBoxLayout* setStartBox = new QHBoxLayout(0);
        gotoLayout->addLayout(setStartBox);

        setStartBox->addSpacing(12);
        setStartBox->addWidget(new QLabel("Time:", group));
        setStartBox->addWidget(setStart_time = new SpinBoxWithFocusReport(group));
       
        setRange = new QRadioButton("Set range", group);
        setRange->setObjectName("setRange");
        gotoLayout->addWidget(setRange);

        QGridLayout* setRangeBox = new QGridLayout(0);
        gotoLayout->addLayout(setRangeBox);
        setRangeBox->setColumnMinimumWidth(0, 12);
        setRangeBox->addWidget(new QLabel("Start:", group), 0, 1);
        setRangeBox->addWidget(setRange_begin = new SpinBoxWithFocusReport(group), 0, 2);
        setRangeBox->addWidget(new QLabel("End:", group), 1, 1);
        setRangeBox->addWidget(setRange_end = new SpinBoxWithFocusReport(group), 1, 2);

        showAllTrace = new QRadioButton("Show all trace", group);
        showAllTrace->setObjectName("showAllTrace");
        gotoLayout->addWidget(showAllTrace);

        errorMessage = new QLabel(group);
        gotoLayout->addWidget(errorMessage);

        gotoLayout->addStretch();

        QHBoxLayout* buttons = new QHBoxLayout(0);
        gotoLayout->addLayout(buttons);
        buttons->addStretch();
        
        okButton = new QPushButton("OK", group);
        buttons->addWidget(okButton);

        QPushButton* reset = new QPushButton("Reset", group);
        buttons->addWidget(reset);

        connect(okButton, SIGNAL(clicked(bool)),
                this, SLOT(done()));

        connect(reset, SIGNAL(clicked(bool)),
                this,  SLOT(reset()));

        connect(setStart, SIGNAL(toggled(bool)),
                this, SLOT(checkboxToggled(bool)));

        connect(setRange, SIGNAL(toggled(bool)),
                this, SLOT(checkboxToggled(bool)));

        connect(showAllTrace, SIGNAL(toggled(bool)),
                this, SLOT(checkboxToggled(bool)));

        connect(setStart_time, SIGNAL(focusIn()),
                this, SLOT(spinButtonFocusIn()));

        connect(setRange_begin, SIGNAL(focusIn()),
                this, SLOT(spinButtonFocusIn()));

        connect(setRange_end, SIGNAL(focusIn()),
                this, SLOT(spinButtonFocusIn()));

        connect(setRange_begin, SIGNAL(valueChanged(int)),
                this, SLOT(validate()));


        connect(setRange_end, SIGNAL(valueChanged(int)),
                this, SLOT(validate()));
        


        QSettings settings;
        QString checked = settings.value("goto sidebar/checkbox").toString();
        if (checked.isEmpty())
        {
            setStart->setChecked(true);
        }
        else
        {
            if (QRadioButton* rb = findChild<QRadioButton*>(checked))
            {
                rb->setChecked(true);
            }
            else
            {
                setStart->setChecked(true);
            }
        }

        

    }

    QAction* createAction()
    {
        QAction* goto_ = new QAction(QIcon(":/link.png"), "&Go to", this);
        goto_->setObjectName("goto_action");
        goto_->setShortcut(QKeySequence(Qt::Key_G));
        return goto_;
    }

    void activate()
    {
        Trace_model* root = model()->root();

        setStart_time->setMinimum(root->min_time());
        setRange_begin->setMinimum(root->min_time());
        setRange_end->setMinimum(root->min_time());

        setStart_time->setMaximum(root->max_time());
        setRange_begin->setMaximum(root->max_time());
        setRange_end->setMaximum(root->max_time());

        setStart_time->setValue(model()->min_time());
        setRange_begin->setValue(model()->min_time());        
        setRange_end->setValue(model()->max_time());

    }

    void deactivate()
    {
    }


private slots:

    void done()
    {
        if (setRange->isChecked())
        {
            Trace_model* n = model()->set_range(
                setRange_begin->value(), setRange_end->value());

            canvas()->setModel(n);
        }
        else if (showAllTrace->isChecked())
        {
            Trace_model* root = model()->root();

            Trace_model* n = model()->set_range(
                root->min_time(), root->max_time());
            canvas()->setModel(n);
        }

        emit browse();
    }

    void reset()
    {
        activate();
    }


    void checkboxToggled(bool checked)
    {
        QSettings settings;
        if (checked)
        {
            if (sender())
            {
                settings.setValue("goto sidebar/checkbox", 
                                  sender()->objectName());
            }
        }
    }

    void spinButtonFocusIn()
    {
        if (sender() == setStart_time)
            setStart->setChecked(true);
        else if (sender() == setRange_begin || sender() == setRange_end)
            setRange->setChecked(true);
    }

    void validate()
    {
        if (setRange_begin->value() > setRange_end->value())
        {
            errorMessage->setText("<b>Error: start > end</b>");
            okButton->setEnabled(false);
        }
        else
        {
            errorMessage->setText("");
            okButton->setEnabled(true);
        }
    }

private:

    void keyReleaseEvent(QKeyEvent* event) 
    {
        if (event->key() == Qt::Key_Enter)
        {
            if (okButton->isEnabled())
                okButton->animateClick();
        }
        else
        {
            Tool::keyReleaseEvent(event);
        }
    }

    QRadioButton* setStart;
    QRadioButton* setRange;
    QRadioButton* showAllTrace;

    QSpinBox* setStart_time;
    QSpinBox* setRange_begin;
    QSpinBox* setRange_end;

    QLabel* errorMessage;
    QPushButton* okButton;
};

Tool* createGoto(QWidget* parent, Canvas* canvas)
{
    return new Goto(parent, canvas);
}

#include "goto.moc"
