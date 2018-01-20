#include "subjectsdbinterface.h"

SubjectsDBInterface::SubjectsDBInterface(QSqlDatabase *db) : DBInterface<Subject>(db)
{
    m_attributes.append(DBAttribute("name", "", &Subject::getName, &Subject::setName));
    m_attributes.append(DBAttribute("shortName", "", &Subject::getShortName, &Subject::setShortName));
    m_attributes.append(DBAttribute("color", "", &Subject::getColor, &Subject::setColor));
    m_attributes.append(DBAttribute("weight", "", &Subject::getWeight, &Subject::setWeight));
}
