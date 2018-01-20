#include "groupsdbinterface.h"

GroupsDBInterface::GroupsDBInterface(QSqlDatabase *db) : DBInterface<Group>(db)
{
    m_attributes.append(DBAttribute("name", "", &Group::getName,  &Group::setName));
}
