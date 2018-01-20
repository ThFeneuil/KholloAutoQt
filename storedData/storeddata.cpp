#include "storeddata.h"

StoredData::StoredData()
{
    m_id = 0;
}

//Getters
int StoredData::getId() const {
    return m_id;
}

//Setters
void StoredData::setId(int id) {
    m_id = id;
}
