#include "studentgrouplinksdbinterface.h"

StudentGroupLinksDBInterface::StudentGroupLinksDBInterface(QSqlDatabase *db) : DBInterface<StudentGroupLink>(db)
{
    m_attributes.append(DBAttribute("id_groups", GROUPS_TABLE, &StudentGroupLink::getId_groups, &StudentGroupLink::setId_groups));
    m_attributes.append(DBAttribute("id_users", STUDENTS_TABLE, &StudentGroupLink::getId_students, &StudentGroupLink::setId_students));
}
