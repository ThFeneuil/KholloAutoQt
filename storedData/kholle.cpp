#include "kholle.h"

Kholle::Kholle()
{
    m_id = 0;
    m_id_students = 0;
    m_id_timeslots = 0;
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
