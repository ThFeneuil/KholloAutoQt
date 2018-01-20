#ifndef STUDENTSDBINTERFACE_H
#define STUDENTSDBINTERFACE_H

#include "DB/dbinterface.h"
#include "storedData/student.h"

#define STUDENTS_TABLE "tau_users"

class StudentsDBInterface : public DBInterface<Student>
{
public:
    StudentsDBInterface(QSqlDatabase *db);
    virtual QString table_name() { return STUDENTS_TABLE; }
};

#endif // STUDENTSDBINTERFACE_H
