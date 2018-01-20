#ifndef STUDENTGROUPLINK_H
#define STUDENTGROUPLINK_H

#include <QVariant>
#include "storeddata.h"

class StudentGroupLink : public StoredData
{
public:
    StudentGroupLink();

    //Getters
    int getId_students() const;
    int getId_groups() const;

    //Setters
    void setId_students(int id_students);
    void setId_students(QVariant id_students);
    void setId_groups(int id_groups);
    void setId_groups(QVariant id_groups);

private:
    int m_id_students;
    int m_id_groups;
};

#endif // STUDENTGROUPLINK_H
