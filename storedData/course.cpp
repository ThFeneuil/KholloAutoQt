#include "storedData/course.h"

Course::Course()
{
    m_id = 0;
    m_id_subjects = 0;
    m_id_groups = 0;
    m_id_day = 0;
    m_id_week = 0;

    m_subject = NULL;
    m_group = NULL;
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

QTime Course::getTime_start() const {
    return m_time_start;
}

QTime Course::getTime_end() const {
    return m_time_end;
}

int Course::getId_groups() const {
    return m_id_groups;
}
int Course::getId_day() const {
    return m_id_day;
}

int Course::getId_week() const {
    return m_id_week;
}
Subject* Course::subject() const {
    return m_subject;
}
Group* Course::group() const {
    return m_group;
}


//Setters
void Course::setId(int id) {
    m_id = id;
}

void Course::setId_subjects(int id_subjects) {
    m_id_subjects = id_subjects;
}

void Course::setTime_start(QTime time_start) {
    m_time_start = time_start;
}

void Course::setTime_end(QTime time_end) {
    m_time_end = time_end;
}

void Course::setId_groups(int id_groups) {
    m_id_groups = id_groups;
}
void Course::setId_day(int id_day) {
    m_id_day = id_day;
}

void Course::setId_week(int id_week) {
    m_id_week = id_week;
}
void Course::setSubject(Subject* subj) {
    m_subject = subj;
}
void Course::setGroup(Group* grp) {
    m_group = grp;
}
