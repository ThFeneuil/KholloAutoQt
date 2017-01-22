#include "timeslot.h"

Timeslot::Timeslot()
{
    m_id = 0;
    m_id_kholleurs = 0;
    m_date = QDate::currentDate();
    m_pupils = 0;
    m_kholles = new QList<Kholle*>();
    m_kholleur = NULL;
}

Timeslot::~Timeslot() {
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
QList<Kholle*>* Timeslot::kholles() const {
    return m_kholles;
}
Kholleur* Timeslot::kholleur() const {
    return m_kholleur;
}

bool Timeslot::isDeleted() const {
    return m_is_deleted;
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

void Timeslot::setIsDeleted(bool is_deleted) {
    m_is_deleted = is_deleted;
}


//Other functions
int Timeslot::weeksTo(Timeslot *ts) {
    /** Number of weeks between the two Timeslot's **/

    //Get the corresponding mondays to have number of weeks between the two dates
    QDate d1 = this->getDate();
    while(d1.dayOfWeek() != 1)
        d1 = d1.addDays(-1);

    QDate d2 = ts->getDate();
    while(d2.dayOfWeek() != 1)
        d2 = d2.addDays(-1);

    return int(abs(d1.daysTo(d2)) / 7);
}
