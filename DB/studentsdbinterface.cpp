#include "studentsdbinterface.h"

StudentsDBInterface::StudentsDBInterface(QSqlDatabase *db) : DBInterface<Student>(db)
{
    m_attributes.append(DBAttribute("name", "", &Student::getName, &Student::setName));
    m_attributes.append(DBAttribute("first_name", "", &Student::getFirst_name, &Student::setFirst_name));
    m_attributes.append(DBAttribute("email", "", &Student::getEmail, &Student::setEmail));
}
