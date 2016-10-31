#include "kholle.h"

Kholle::Kholle()
{
    m_id = 0;
    m_id_students = 0;
    m_id_timeslots = 0;

    m_student = NULL;
    m_timeslot = NULL;
    m_status = OK;
    m_weeks = 0;
}

Kholle::~Kholle() {

}

//Getters
int Kholle::getId() const {
    return m_id;
}

int Kholle::getId_students() const {
    return m_id_students;
}

int Kholle::getId_timeslots() const {
    return m_id_timeslots;
}
Student* Kholle::student() const {
    return m_student;
}
Timeslot* Kholle::timeslot() const {
    return m_timeslot;
}
int Kholle::status() const {
    return m_status;
}
int Kholle::weeks() const {
    return m_weeks;
}


//Setters
void Kholle::setId(int id) {
    m_id = id;
}

void Kholle::setId_students(int id_students) {
    m_id_students = id_students;
}

void Kholle::setId_timeslots(int id_timeslots) {
    m_id_timeslots = id_timeslots;
}
void Kholle::setStudent(Student* stud) {
    m_student = stud;
}
void Kholle::setTimeslot(Timeslot* slot) {
    m_timeslot = slot;
}
void Kholle::setStatus(Status status) {
    m_status = status;
}
void Kholle::setWeeks(int weeks) {
    m_weeks = weeks;
}
