#ifndef SUBJECTSDBINTERFACE_H
#define SUBJECTSDBINTERFACE_H

#include "dbinterface.h"
#include "storedData/subject.h"

#define SUBJECTS_TABLE "tau_subjects"

class SubjectsDBInterface : public DBInterface<Subject>
{
public:
    SubjectsDBInterface(QSqlDatabase *db);
    virtual QString table_name() { return SUBJECTS_TABLE; }
};

#endif // SUBJECTSDBINTERFACE_H
