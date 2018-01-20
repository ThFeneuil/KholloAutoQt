#include "dependencysolver.h"

DependencySolver::DependencySolver()
{

}

void DependencySolver::proper_delete(QSqlDatabase *m_db, QString other_table, int id) {
    /// Add all interfaces here to delete dependencies. For example :
    /// MyDBInterface(m_db).remove_dependencies(other_table, id);

    StudentsDBInterface(m_db).remove_dependencies(other_table, id);
    GroupsDBInterface(m_db).remove_dependencies(other_table, id);
    StudentGroupLinksDBInterface(m_db).remove_dependencies(other_table, id);
    SubjectsDBInterface(m_db).remove_dependencies(other_table, id);
}
