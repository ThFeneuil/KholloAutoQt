#ifndef STUDENTGROUPLINKSDBINTERFACE_H
#define STUDENTGROUPLINKSDBINTERFACE_H

#include "dbinterface.h"
#include "storedData/studentgrouplink.h"

#define STUDENTGROUPLINKS_TABLE "tau_groups_users"

class StudentGroupLinksDBInterface : public DBInterface<StudentGroupLink>
{
public:
    StudentGroupLinksDBInterface(QSqlDatabase *db);
    virtual QString table_name() { return STUDENTGROUPLINKS_TABLE; }
};

#endif // STUDENTGROUPLINKSDBINTERFACE_H
