#include "storedData/student.h"

Student::Student() {
    m_id = 0;
    m_name = "";
    m_first_name = "";
    m_email = "";
}

Student::~Student() {

}

//Getters
int Student::getId() const {
    return m_id;
}
QString Student::getName() const {
    return m_name;
}
QString Student::getFirst_name() const {
    return m_first_name;
}
QString Student::getEmail() const {
    return m_email;
}

//Setters
void Student::setId(int id) {
    m_id = id;
}
void Student::setName(QString name) {
    m_name = name;
}
void Student::setFirst_name(QString first_name) {
    m_first_name = first_name;
}
void Student::setEmail(QString email) {
    m_email = email;
}
