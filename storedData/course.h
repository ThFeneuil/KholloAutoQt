#ifndef COURSE_H
#define COURSE_H

#include <QString>
#include <QTime>

class Course
{
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

    //Setters
    void setId(int id);
    void setId_subjects(int id_subjects);
    void setTime_start(QTime time_start);
    void setTime_end(QTime time_end);
    void setId_groups(int id_groups);
    void setId_teachers(int id_teachers);
    void setId_day(int id_day);
    void setId_week(int id_week);

private:
    int m_id;
    int m_id_subjects;
    QTime m_time_start;
    QTime m_time_end;
    int m_id_groups;
    int m_id_teachers;
    int m_id_day;
    int m_id_week;
};

#endif // COURSE_H
