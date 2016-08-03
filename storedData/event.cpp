#include "storedData/event.h"

Event::Event() {
    m_id = 0;
    m_name = "";
    m_comment = "";
    m_start = QDateTime::currentDateTime();
    m_end = m_start.addDays(1);
    m_groups2 = new QList<Group*>();
    m_groups = new QList<Group*>();
}

Event::~Event() {
    for(int i=0; i<m_groups->count(); i++)
        delete m_groups->at(i);
    delete m_groups2;
    delete m_groups;
}

//Getters
int Event::getId() const {
    return m_id;
}
QString Event::getName() const {
    return m_name;
}
QString Event::getComment() const {
    return m_comment;
}
QDateTime Event::getStart() const {
    return m_start;
}
QDateTime Event::getEnd() const {
    return m_end;
}
QList<Group*>* Event::getGroups() const {
    return m_groups2;
}
QList<Group*>* Event::groups() const {
    return m_groups;
}

//Setters
void Event::setId(int id) {
    m_id = id;
}
void Event::setName(QString name) {
    m_name = name;
}
void Event::setComment(QString comment) {
    m_comment = comment;
}
void Event::setStart(QDateTime start) {
    m_start = start;
}
void Event::setEnd(QDateTime end) {
    m_end = end;
}
void Event::setGroups(QList<Group*>* groups) {
    m_groups2 = groups;
}
