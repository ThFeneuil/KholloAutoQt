#include "storedData/group.h"

Group::Group() {
    m_name = "";
    m_students = new QList<Student*>();
    m_courses = new QList<Course*>();
    m_events = new QList<Event*>();
}

Group::~Group() {
    delete m_students;
    delete m_courses;
    delete m_events;
}

//Getters
QString Group::getName() const {
    return m_name;
}
QList<Student*>* Group::students() const {
    return m_students;
}
QList<Course*>* Group::courses() const {
    return m_courses;
}
QList<Event*>* Group::events() const {
    return m_events;
}

//Setters
void Group::setName(QString name) {
    m_name = name;
}
void Group::setName(QVariant name) {
    setName(name.toString());
}
