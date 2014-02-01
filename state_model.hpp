
#ifndef STATE_MODEL_HPP_VP_2006_03_21
#define STATE_MODEL_HPP_VP_2006_03_21

#include <QString>

/** �������� ��������� ��� ����� ������������. */
class State_model
{
public:
    /** ��������� ����� ���������. */
    unsigned begin;

    /** �������� ����� ���������. */
    unsigned end;
    
    /** ��� ���������. ������������ ������ � ���������� ������������ */
    QString name;
    
    /** ����� ����������, � �������� ��������� ���������. */
    unsigned component;
    
    virtual ~State_model() {}
};

#endif
