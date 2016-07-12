#include "storedData/subject.h"

Subject::Subject() {
    m_id = 0;
    m_name = "";
    m_shortName = "";
    m_color = "";
}

Subject::~Subject() {

}

//Getters
int Subject::getId() const {
    return m_id;
}
QString Subject::getName() const {
    return m_name;
}
QString Subject::getShortName() const {
    return m_shortName;
}
QString Subject::getColor() const {
    return m_color;
}

//Setters
void Subject::setId(int id) {
    m_id = id;
}
void Subject::setName(QString name) {
    m_name = name;
}
void Subject::setShortName(QString shortName) {
    m_shortName = shortName;
}
void Subject::setColor(QString color) {
    m_color = color;
}
