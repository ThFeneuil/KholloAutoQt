#include "timeslot.h"

Timeslot::Timeslot()
{
    m_id = 0;
    m_id_kholleurs = 0;
    m_id_day = 0;
    m_pupils = 0;
}

Timeslot::~Timeslot() {

}

//Getters
int Timeslot::getId() const {
    return m_id;
}

QTime Timeslot::getTime_start() const {
    return m_time_start;
}

QTime Timeslot::getTime() const {
    return m_time;
}

QTime Timeslot::getTime_end() const {
    return m_time_end;
}

int Timeslot::getId_kholleurs() const {
    return m_id_kholleurs;
}

int Timeslot::getId_day() const {
    return m_id_day;
}

int Timeslot::getPupils() const {
    return m_pupils;
}


//Setters
void Timeslot::setId(int id) {
    m_id = id;
}

void Timeslot::setTime_start(QTime time_start) {
    m_time_start = time_start;
}

void Timeslot::setTime(QTime time) {
    m_time = time;
}

void Timeslot::setTime_end(QTime time_end) {
    m_time_end = time_end;
}

void Timeslot::setId_kholleurs(int id_kholleurs) {
    m_id_kholleurs = id_kholleurs;
}

void Timeslot::setId_day(int id_day) {
    m_id_day = id_day;
}

void Timeslot::setPupils(int pupils) {
    m_pupils = pupils;
}
