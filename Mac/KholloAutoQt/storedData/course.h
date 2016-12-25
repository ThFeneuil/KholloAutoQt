#ifndef COURSE_H
#define COURSE_H

#include <QString>
#include <QTime>
#include "storedData/subject.h"
#include "storedData/group.h"
#include "storedData/teacher.h"

class Subject;
class Group;
class Teacher;

class Course {
public:
    Course();
    ~Course();

    //Getters
    int getId() const;
    int getId_subjects() const;
    QTime getTime_start() const;
    QTime getTime_end() const;
    int getId_groups() const;
    int getId_teachers() const;
    int getId_day() const;
    int getId_week() const;
    Subject* subject() const;  // Interface
    Group* group() const;  // Interface
    Teacher* teacher() const;  // Interface

    //Setters
    void setId(int id);
    void setId_subjects(int id_subjects);
    void setTime_start(QTime time_start);
    void setTime_end(QTime time_end);
    void setId_groups(int id_groups);
    void setId_teachers(int id_teachers);
    void setId_day(int id_day);
    void setId_week(int id_week);
    void setSubject(Subject* subj);  // Interface
    void setGroup(Group* grp);  // Interface
    void setTeacher(Teacher* tcher);  // Interface

private:
    int m_id;
    int m_id_subjects;
    QTime m_time_start;
    QTime m_time_end;
    int m_id_groups;
    int m_id_teachers;
    int m_id_day;
    int m_id_week;

    // Interface
    Subject* m_subject;
    Group* m_group;
    Teacher* m_teacher;
};

#endif // COURSE_H
