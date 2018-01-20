#ifndef DEPENDENCYSOLVER_H
#define DEPENDENCYSOLVER_H

#include <QString>
#include <QtSql>

class DependencySolver
{
public:
    DependencySolver();
    static void proper_delete(QSqlDatabase *m_db, QString other_table, int id);
};

/** *********************************
 * Add the DBInterface's here !    *   <-----------------
 * ********************************/
#include "studentsdbinterface.h"
#include "groupsdbinterface.h"
#include "studentgrouplinksdbinterface.h"
#include "subjectsdbinterface.h"

#endif // DEPENDENCYSOLVER_H
