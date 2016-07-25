#include "kholle.h"

Kholle::Kholle()
{
    m_id = 0;
    m_id_subjects = 0;
    m_id_students = 0;
    m_id_teachers = 0;
}

Kholle::~Kholle() {

}

//Getters
int Kholle::getId() const {
    return m_id;
}

QDateTime Kholle::getTime_start() const {
    return m_time_start;
}

QDateTime Kholle::getTime() const {
    return m_time;
}

QDateTime Kholle::getTime_end() const {
    return m_time_end;
}

int Kholle::getId_subjects() const {
    return m_id_subjects;
}

int Kholle::getId_students() const {
    return m_id_students;
}

int Kholle::getId_teachers() const {
    return m_id_teachers;
}


//Setters
void Kholle::setId(int id) {
    m_id = id;
}

void Kholle::setTime_start(QDateTime time_start) {
    m_time_start = time_start;
}

void Kholle::setTime(QDateTime time) {
    m_time = time;
}

void Kholle::setTime_end(QDateTime time_end) {
    m_time_end = time_end;
}

void Kholle::setId_subjects(int id_subjects) {
    m_id_subjects = id_subjects;
}

void Kholle::setId_students(int id_students) {
    m_id_students = id_students;
}

void Kholle::setId_teachers(int id_teachers) {
    m_id_teachers = id_teachers;
}
