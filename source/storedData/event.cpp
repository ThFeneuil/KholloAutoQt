#include "storedData/event.h"

Event::Event() {
    m_name = "";
    m_comment = "";
    m_start = QDateTime::currentDateTime();
    m_end = m_start.addDays(1);
    m_groups = new QList<Group*>();
}

Event::~Event() {
    delete m_groups;
}

//Getters
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
QList<Group*>* Event::groups() const {
    return m_groups;
}

//Setters
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
