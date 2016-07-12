#include "storedData/kholleur.h"

Kholleur::Kholleur() {
    m_id = 0;
    m_name = "";
    m_id_subjects = 0;
    m_duration = 0;
    m_preparation = 0;
    m_pupils = 0;
}

Kholleur::~Kholleur() {

}

//Getters
int Kholleur::getId() const {
    return m_id;
}
QString Kholleur::getName() const {
    return m_name;
}
int Kholleur::getId_subjects() const {
    return m_id_subjects;
}
int Kholleur::getDuration() const {
    return m_duration;
}
int Kholleur::getPreparation() const {
    return m_preparation;
}
int Kholleur::getPupils() const {
    return m_pupils;
}

// Setters
void Kholleur::setId(int id) {
    m_id = id;
}
void Kholleur::setName(QString name) {
    m_name = name;
}
void Kholleur::setId_subjects(int id_subjects) {
    m_id_subjects = id_subjects;
}
void Kholleur::setDuration(int duration) {
    m_duration = duration;
}
void Kholleur::setPreparation(int preparation) {
    m_preparation = preparation;
}
void Kholleur::setPupils(int pupils) {
    m_pupils = pupils;
}

