#include "storedData/student.h"

Student::Student() {
    m_name = "";
    m_first_name = "";
    m_email = "";
    m_groups = new QList<Group*>();
    m_kholles = new QList<Kholle*>();
}

Student::~Student() {
    delete m_groups;
    delete m_kholles;
}

//Getters
QString Student::getName() const {
    return m_name;
}
QString Student::getFirst_name() const {
    return m_first_name;
}
QString Student::getEmail() const {
    return m_email;
}
QList<Group*>* Student::groups() const {
    return m_groups;
}
QList<Kholle*>* Student::kholles() const {
    return m_kholles;
}

//Setters
void Student::setName(QString name) {
    m_name = name;
}
void Student::setName(QVariant name) {
    setName(name.toString());
}
void Student::setFirst_name(QString first_name) {
    m_first_name = first_name;
}
void Student::setFirst_name(QVariant first_name) {
    setFirst_name(first_name.toString());
}
void Student::setEmail(QString email) {
    m_email = email;
}
void Student::setEmail(QVariant email) {
    setEmail(email.toString());
}
