
#ifndef TRACE_MODEL_HPP_VP_2006_03_20
#define TRACE_MODEL_HPP_VP_2006_03_20

class Trace;
class Event_model;
class State_model;
class Group_model;

#include <memory>

/** ���������� ������������� ������ ��� �������������. 

    ���� ����������� ����� ��������� ��, ��� ������ ���� �������� �� ������ (����� �����,
    �������, ���������), � ������������� ���������� ��� ��������� �� �������� ������������. 
    ����� ����������� ������������ ������ ������������ ��������������� ��������, ������
    ����� �� ������������ ������ �����������.

    ��������� ���� ������ � ������ ������ Vis3 �������� ����� ���� ���������, ������� ��� ��������������
    ������������.

    ��� ������-������� ������ ���� "�������", ����� ������������ ������ �� ������ �����������
    ������ ���� �����������, � �����������/��������� ������ ���� �������� ����������. ��� ��������,
    ��� ��� ������������� ������� �������� ������ ��� ������������� ������ ���������� �������-��������
    ����� ��������� �� ���������� � ������, ������� ������ ���� ������ ��������� �������/�������.

    ��������������� ���������� �� ���� ����������� �������� ������. ��� ������ ��������� ������� ����� 
    ���������. ���, � ��������� � ����������� "��������" ��������, ��������� ��������� �������, �����
    ����� � ��� ����� ���� ������ ���������. ��� �������, ��������, ��� �������� "back".

    ��������� ����������� �� ���� ����.

    ������ ��� -- ��������� � ��������� ���������. ��������������, ��� ���� �������� ����� �������
    � ��������� �����������. ��� ������ ������� ���� ����� ������������ ������� ����� �����, �������
    � ���������. ����� ����� ������� ������������ ���������� ����������� ���������� ����������
    ���������� Trace_model.

    ����� ��������� �������� �������� ���� �����
    - ������� "������"
    - "���������" ����� �� ����� ����� � ������ � ����
    ��� ������� ��������� ����������� ���������� ���� ��������� ���������� ���������� ���������� 
    ���������� Trace_model.

    ���������� ��������, ��� "������" � "���������" -- �������� �������, � �� ������������ ���������
    ���������. �������� �������, ����� �� ��� ����� ������� �� ���������� �������� ������ �
    ����, � ���� ������ ��� ��������� ������ �������� �������-����������� ��������. ����� �������� �������� --
    ����� ���� �������� ������ �����������, ��������� �� �������� �������������� � ������� ���������
    ����� � ����, � ��������� "�����������" ������������� ������, ������� �� ������� ����������
    �������-����������� ���������.

    ������ ��� ��������� -- ������� ����� �����, ������� � ���������. ���� ��������� � ���������
    ��������� ���������� ������������� ����� ���� ��������, �� �������������� ���������� ����������
    ������� ������� �����.

    ������ ��� ��������� -- ��� ����������� � ������������ ������������ �����. ���� ���� ������ �
    ������������ ������� -- ����� � ������ ������� ��������� ���������. �������, ������������ Trace_model,
    ����� �������� �� ���� ��������, ��� �������� ��������� ��� "�����������" ���������
    (��������� � ��������� �������). ����� �������, ��� ��� ������� �������� �� ����������� ���������, 
    ����� ���� ��������� ���������� ��������� ��������� �������. 

    ��������� �� ���� ���� ���� ����������� � ������� ������� ������� ������ Trace_model, �������
    ������� ����� ������ ����� Trace_model � ����� �������������� ������. ��������, ��� ���������
    ����� �� ����� ����� ���������� ����� enter_component � ������� ����� �����, ��� ��������� �����
    ����������� ���������� ����� filters_components c ������� ������. ��� ������ ��� ���������
    ������� ������ ������� ������������ "�������" -- �� ������� ������� ��������, � ����������������
    ���������, � � ������������� ��������� �������.
*/
class Trace_model 
{
public:
    /** Returns the name of the parent component,
        or empty if there's no parent. */
    virtual std::string parent_name() const = 0;

    /** ���������� ������ ���� ���� ������� ���������. 
    
        ����� ��������� ������ ���� �����������, ��� ��� ������������ �����
        ����� �������� ������������. */
    virtual const std::vector<QString>& component_names() const = 0;

    /** ���������� true ���� �������� i ����� "�����" � � ���� ����� ������.
        @sa enter_component. */
    virtual bool has_children(int i) const = 0;

    /** ���������� ����������� �����, ������� ������ ������������ �� ��������� ���������. */
    virtual unsigned min_time() const = 0;

    /** ���������� ������������ �����, ������� ������ ������������ �� ��������� ���������. */
    virtual unsigned max_time() const = 0;

    /** ��������� ���������� ��������� �������, ��������� � ��������� �������
       �� ����������� �����. */
    virtual void rewind() = 0;

    /** ���������� ��������� ������� � ������ � ����������� ���������� ���������
        �������. ���� ������ ������� ���, ���������� ������� ���������. */
    virtual std::auto_ptr<Event_model> next_event() = 0;

    /** ���������� ��������� ��������� � ������ � ����������� ���������� ���������
    �������. ���� ������ ������� ���, ���������� ������� ���������. */
    virtual std::auto_ptr<State_model> next_state() = 0;

    /** ���������� ��������� ��������� � ������ � ����������� ���������� ���������
    �������. ���� ������ ������� ���, ���������� ������� ���������. */
    virtual std::auto_ptr<Group_model> next_group() = 0;    

    /** ���������� ������ Trace_model ��������������� "����������" ���������
    � ��������� ����������, ������������� ���������� ��������� ������ �
    �� ������� ������� ��������. */
    virtual Trace_model* root() = 0;

    /** ���������� Trace_model, ������������ i-� ��������� this, � "���������"
    ����. */
    virtual Trace_model* enter_component(int i) = 0;

    /** ���������� ����� ������ Trace_model � ��������� ���������� ������. */
    virtual Trace_model* set_range(unsigned min, unsigned max) = 0;

    /** ������� ����� ������, ������� �������� ������ �� ����������, ���
       ������� � ������� allow ����� true. ������ ������� ������ ���� �����
       component_names().size(). */
    virtual Trace_model* filter_components(const QVector<bool>& allow) = 0;

    /** ���������� "������������" ��������� ������������� ��������� � ��������� ����������.

        ������������� ����� �������������� ������ ��� ������������� �����-���� ��������������
        ������. ��������� �������������� �� ���������������.
    */
    QString component_tree_location() const;

    /** ���������� ������ Trace_model, ������� ���������� �� this ������ ���������� �����-����
        �������� �� �����������/��������/����������. */
    virtual Trace_model* unfiltered() = 0;

    /** ���������� ������ ����� �������. ��� ���� ������ ���� �����������. */
    virtual std::vector<QString> event_kinds() = 0;

    /** ������� ����� ������, ������� �������� ������� ������ ��� �����, ���
        ������� � ������� allow ����� true. ������ ������� ������ ���� �����
        event_kinds().size(). */
    virtual Trace_model* filter_events(const QVector<bool>& allow) = 0;

    /** ��������� ������� ��������� � ������ � ���������� ���. ��������� ������ ��
       ����������. */
    virtual QString save() const = 0;

    /** �� ������, ����� ������������ �������� save, �������� ������������ 
       ���������. ������� �� ����������� ���-�� ������, ���� �������� ������ ������ 
       ������� ���������� �� ������ ������ � ������ ������ save, ������� ����� ������
       �� ������.

       �������� �������������� -- ��� �������������� ��������� ������������ ����������
       ��� ����������� ��������. */
    virtual void restore(const QString& s) = 0;

    virtual ~Trace_model() {}

};


#endif
