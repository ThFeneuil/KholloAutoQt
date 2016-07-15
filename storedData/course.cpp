#include "storedData/course.h"

Course::Course()
{
    m_id = 0;
    m_id_subjects = 0;
    m_time_start = "";
    m_time_end = "";
    m_id_groups = 0;
    m_id_teachers = 0;
    m_id_day = 0;
    m_id_week = 0;
}

Course::~Course() {

}

//Getters
int Course::getId() const {
    return m_id;
}

int Course::getId_subjects() const {
    return m_id_subjects;
}

QString Course::getTime_start() const {
    return m_time_start;
}

QString Course::getTime_end() const {
    return m_time_end;
}

int Course::getId_groups() const {
    return m_id_groups;
}

int Course::getId_teachers() const {
    return m_id_teachers;
}

int Course::getId_day() const {
    return m_id_day;
}

int Course::getId_week() const {
    return m_id_week;
}


//Setters
void Course::setId(int id) {
    m_id = id;
}

void Course::setId_subjects(int id_subjects) {
    m_id_subjects = id_subjects;
}

void Course::setTime_start(QString time_start) {
    m_time_start = time_start;
}

void Course::setTime_end(QString time_end) {
    m_time_end = time_end;
}

void Course::setId_groups(int id_groups) {
    m_id_groups = id_groups;
}

void Course::setId_teachers(int id_teachers) {
    m_id_teachers = id_teachers;
}

void Course::setId_day(int id_day) {
    m_id_day = id_day;
}

void Course::setId_week(int id_week) {
    m_id_week = id_week;
}
