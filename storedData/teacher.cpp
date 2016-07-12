#include "storedData/teacher.h"

Teacher::Teacher() {
    m_id = 0;
    m_name = "";
    m_id_subjects = 0;
}

Teacher::~Teacher() {

}

//Getters
int Teacher::getId() const {
    return m_id;
}
QString Teacher::getName() const {
    return m_name;
}
int Teacher::getId_subjects() const {
    return m_id_subjects;
}

//Setters
void Teacher::setId(int id) {
    m_id = id;
}
void Teacher::setName(QString name) {
    m_name = name;
}
void Teacher::setId_subjects(int id_subjects) {
    m_id_subjects = id_subjects;
}
