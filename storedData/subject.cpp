#include "storedData/subject.h"

Subject::Subject() {
    m_name = "";
    m_shortName = "";
    m_color = "";
    m_weight = 0;
    m_kholleurs = new QList<Kholleur*>();
    m_courses = new QList<Course*>();
}

Subject::~Subject() {
    delete m_kholleurs;
    delete m_courses;
}

//Getters
QString Subject::getName() const {
    return m_name;
}
QString Subject::getShortName() const {
    return m_shortName;
}
QString Subject::getColor() const {
    return m_color;
}
int Subject::getWeight() const {
    return m_weight;
}
QList<Kholleur*>* Subject::kholleurs() const {
    return m_kholleurs;
}
QList<Course*>* Subject::courses() const {
    return m_courses;
}

//Setters
void Subject::setName(QString name) {
    m_name = name;
}
void Subject::setName(QVariant name) {
    setName(name.toString());
}
void Subject::setShortName(QString shortName) {
    m_shortName = shortName;
}
void Subject::setShortName(QVariant shortName) {
    setShortName(shortName.toString());
}
void Subject::setColor(QString color) {
    m_color = color;
}
void Subject::setColor(QVariant color) {
    setColor(color.toString());
}
void Subject::setWeight(int weight) {
    if(weight < 0)
        m_weight = 0;
    else
        m_weight = (weight > MaxWeightSubject) ? MaxWeightSubject : weight;
}
void Subject::setWeight(QVariant weight) {
    setWeight(weight.toInt());
}
