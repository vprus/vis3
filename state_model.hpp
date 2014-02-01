
#ifndef STATE_MODEL_HPP_VP_2006_03_21
#define STATE_MODEL_HPP_VP_2006_03_21

#include <QString>

/** Описание состояния для целей визуализации. */
class State_model
{
public:
    /** Начальное время состояния. */
    unsigned begin;

    /** Конечное время состояния. */
    unsigned end;
    
    /** Имя состояния. Используется только в сообщениях пользователю */
    QString name;
    
    /** Номер компонента, к которому относится состояние. */
    unsigned component;
    
    virtual ~State_model() {}
};

#endif
