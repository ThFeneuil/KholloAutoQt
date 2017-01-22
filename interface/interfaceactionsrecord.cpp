#include "interfaceactionsrecord.h"

InterfaceActionsRecord::InterfaceActionsRecord() {
    m_actions = new QStack<InterfaceAction*>();
}

InterfaceActionsRecord::~InterfaceActionsRecord() {
    delete m_actions;
}

InterfaceAction::InterfaceAction(ActionType type) {
    m_type = type;
    m_param1 = 0;
    m_param2 = 0;
    m_param3 = 0;
}

InterfaceAction::~InterfaceAction() {

}

InterfaceAction::ActionType InterfaceAction::type() {
    return m_type;
}

void InterfaceAction::setAddKholle(Student* stud, Timeslot* ts) {
    m_type = AddKholle;
    m_param1 = (qlonglong) stud;
    m_param2 = (qlonglong) ts;
    m_param3 = 0;
}

void InterfaceAction::setDeleteKholle(Student* stud, Timeslot* ts) {
    m_type = DeleteKholle;
    m_param1 = (qlonglong) stud;
    m_param2 = (qlonglong) ts;
    m_param3 = 0;
}

void InterfaceAction::setCommuteKholle(Student* stud1, Student* stud2, Subject* subj) {
    m_type = CommuteKholle;
    m_param1 = (qlonglong) stud1;
    m_param2 = (qlonglong) stud2;
    m_param3 = (qlonglong) subj;
}

Student* InterfaceAction::student(int nb) const {
    switch(m_type) {
        case AddKholle:
        case DeleteKholle:
            return (Student*) m_param1;
            break;
        case CommuteKholle:
            if(nb == 1) return (Student*) m_param1;
            if(nb == 2) return (Student*) m_param2;
            break;
        default:
            return NULL;
            break;
    }
    return NULL;
}

Timeslot* InterfaceAction::timeslot() const {
    switch(m_type) {
        case AddKholle:
        case DeleteKholle:
            return (Timeslot*) m_param2;
            break;
        default:
            return NULL;
            break;
    }
    return NULL;
}

Subject* InterfaceAction::subject() const {
    if(m_type == CommuteKholle)
        return (Subject*) m_param3;
    return NULL;
}

