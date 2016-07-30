#include "storedData/teacher.h"

Teacher::Teacher() {
    m_id = 0;
    m_name = "";
    m_id_subjects = 0;
    m_subject = NULL;
    m_courses = new QList<Course*>();
}

Teacher::~Teacher() {
    delete m_courses;
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
Subject* Teacher::subject() const {
    return m_subject;
}
QList<Course*>* Teacher::courses() const {
    return m_courses;
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
void Teacher::setSubject(Subject* subj) {
    m_subject = subj;
}
