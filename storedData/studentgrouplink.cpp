#include "studentgrouplink.h"

StudentGroupLink::StudentGroupLink()
{
    m_id_groups = 0;
    m_id_students = 0;
}

//Getters
int StudentGroupLink::getId_groups() const {
    return m_id_groups;
}
int StudentGroupLink::getId_students() const {
    return m_id_students;
}

//Setters
void StudentGroupLink::setId_groups(int id_groups) {
    m_id_groups = id_groups;
}
void StudentGroupLink::setId_groups(QVariant id_groups) {
    setId_groups(id_groups.toInt());
}
void StudentGroupLink::setId_students(int id_students) {
    m_id_students = id_students;
}
void StudentGroupLink::setId_students(QVariant id_students) {
    setId_students(id_students.toInt());
}
