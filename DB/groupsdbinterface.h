#ifndef GROUPSDBINTERFACE_H
#define GROUPSDBINTERFACE_H

#include "DB/dbinterface.h"
#include "storedData/group.h"

#define GROUPS_TABLE "tau_groups"

class GroupsDBInterface : public DBInterface<Group>
{
public:
    GroupsDBInterface(QSqlDatabase *db);
    virtual QString table_name() { return GROUPS_TABLE; }
};

#endif // GROUPSDBINTERFACE_H
