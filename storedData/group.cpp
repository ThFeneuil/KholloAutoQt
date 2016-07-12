#include "storedData/group.h"

Group::Group() {
    m_id = 0;
    m_name = "";
}

Group::~Group() {

}

//Getters
int Group::getId() const {
    return m_id;
}
QString Group::getName() const {
    return m_name;
}

//Setters
void Group::setId(int id) {
    m_id = id;
}
void Group::setName(QString name) {
    m_name = name;
}

