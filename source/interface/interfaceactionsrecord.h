#ifndef INTERFACEACTIONSRECORD_H
#define INTERFACEACTIONSRECORD_H

#include <QStack>
#include "storedData/student.h"

class InterfaceAction {
public:
    enum ActionType { None, AddKholle, DeleteKholle, CommuteKholle};

    InterfaceAction(ActionType type = None);
    ~InterfaceAction();
    ActionType type();
    void setAddKholle(Student* stud, Timeslot* ts);
    void setDeleteKholle(Student* stud, Timeslot* ts);
    void setCommuteKholle(Student* stud1, Student* stud2, Subject *subj);
    Student* student(int nb = 1) const;
    Timeslot* timeslot() const;
    Subject* subject() const;

private:
    ActionType m_type;
    qlonglong m_param1;
    qlonglong m_param2;
    qlonglong m_param3;
};

class InterfaceActionsRecord
{
public:
    InterfaceActionsRecord();
    ~InterfaceActionsRecord();

private:
    QStack<InterfaceAction*>* m_actions;
};

#endif // INTERFACEACTIONSRECORD_H
