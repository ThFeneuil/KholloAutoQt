#include "timeslot.h"

Timeslot::Timeslot()
{
    m_id = 0;
    m_id_kholleurs = 0;
    m_date = QDate::currentDate();
    m_pupils = 0;
    m_kholles2 = new QList<Kholle>();
    m_kholles = new QList<Kholle*>();
    m_kholleur = NULL;
}

Timeslot::~Timeslot() {
    delete m_kholles2;
    delete m_kholles;
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
QDate Timeslot::getDate() const {
    return m_date;
}

int Timeslot::getPupils() const {
    return m_pupils;
}
QRect* Timeslot::getArea() const {
    return m_area;
}
QList<Kholle>* Timeslot::getKholles() const {
    return m_kholles2;
}
QList<Kholle*>* Timeslot::kholles() const {
    return m_kholles;
}
Kholleur* Timeslot::kholleur() const {
    return m_kholleur;
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

void Timeslot::setDate(QDate date) {
    m_date = date;
}

void Timeslot::setPupils(int pupils) {
    m_pupils = pupils;
}

void Timeslot::setArea(QRect* area) {
    m_area = area;
}
void Timeslot::setKholleur(Kholleur* kll) {
    m_kholleur = kll;
}
