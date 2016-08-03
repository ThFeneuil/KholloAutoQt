#include "storedData/subject.h"

Subject::Subject() {
    m_id = 0;
    m_name = "";
    m_shortName = "";
    m_color = "";
    m_teachers = new QList<Teacher*>();
    m_kholleurs = new QList<Kholleur*>();
    m_courses = new QList<Course*>();
}

Subject::~Subject() {
    delete m_teachers;
    delete m_kholleurs;
    delete m_courses;
}

//Getters
int Subject::getId() const {
    return m_id;
}
QString Subject::getName() const {
    return m_name;
}
QString Subject::getShortName() const {
    return m_shortName;
}
QString Subject::getColor() const {
    return m_color;
}
QList<Teacher*>* Subject::teachers() const {
    return m_teachers;
}
QList<Kholleur*>* Subject::kholleurs() const {
    return m_kholleurs;
}
QList<Course*>* Subject::courses() const {
    return m_courses;
}

//Setters
void Subject::setId(int id) {
    m_id = id;
}
void Subject::setName(QString name) {
    m_name = name;
}
void Subject::setShortName(QString shortName) {
    m_shortName = shortName;
}
void Subject::setColor(QString color) {
    m_color = color;
}
