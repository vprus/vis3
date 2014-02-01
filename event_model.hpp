
#ifndef EVENT_MODEL_HPP_VP_2006_03_21
#define EVENT_MODEL_HPP_VP_2006_03_21

#include <QString>
class QWidget;

/** ������ ����� ��������� ������� ��� ����� ������������.

    ��������� �� Event_model ������������ ������� Trace_model::next_event.
    ��������������. ��� ��� ���������� �������� ������������ �������-�������,
    � � ����������� �� �������� ������� ������� ������� ����������, ����� ���
    ������� ������ ���� � ��������� �����.
*/
class Event_model
{
public:
    /** ����� ������������� �������. */
    unsigned time;
    
    /** ��� ������� (������). ������������ ������ ��� ������ ���� 
        ������� � ��������������� ����������.
    */
    QString kind;
    
    /** �����, ������������ ��� ������ �������. �������������� ������
       ������������ ��� ���������. */
    char letter;
    
    /** �������������� �����, ������������ ��� ������ �������. �������������� ������
    ������������ ��� ��������. �������� ������ �� �������� ����� ������� �������. 
    ���� ����� ����, �� ������������. */
    char subletter;
    
    /** ������� ����� ������������ "�������" �������. */
    enum letter_position_t { left_top = 1, right_top, right_bottom, left_bottom} 
    letter_position;
    
    /** ����� ����������, � ������� ��������� �������. ���� ����� ��������
        ��������� ��� ������� Trace_model, �� �������� ���� �������� �������.
    */
    unsigned component;

    /** ����� ����� ���� ���������� ��������������� ��������, ����� ������������
    ����������� ����� ������ ��������� ���������� � �������. ������������ ������
    �� ����������� ������ ����� "����������" ���� ����. ������ ��������� ����������
    ��������. �������������, ��� ������� ��� �������� ��� ������ ������, �� �����
    ������� ������ �������. 
    */
    virtual QWidget* createDetailsWidget(const QWidget* parent) { return 0; }
    
    virtual ~Event_model() {}
    
protected:
    /** ������� ������, ��������� ��� ������ ��������� ��������� ���������� � �������. 
    �������������� ������ ����� ������������ ���� ����� � ����� ���������� createDetailsWidget,
    ���� � ��� ���� ��������� ���������� ��� ������, � ��� ���� �� ��������� ������ ��������
    �� �������������� ���� ����������.
    */
    QWidget* createTextDetailsWidget(const QString& text, const QWidget* parent);
};

#endif
