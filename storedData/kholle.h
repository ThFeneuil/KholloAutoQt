#ifndef Kholle_H
#define Kholle_H

#include <QTime>

class Kholle
{
public:
    Kholle();
    ~Kholle();

    //Getters
    int getId() const;
    QDateTime getTime_start() const;
    QDateTime getTime() const;
    QDateTime getTime_end() const;
    int getId_subjects() const;
    int getId_students() const;
    int getId_teachers() const;

    //Setters
    void setId(int id);
    void setTime_start(QDateTime time_start);
    void setTime(QDateTime time);
    void setTime_end(QDateTime time_end);
    void setId_subjects(int id_subjects);
    void setId_students(int id_students);
    void setId_teachers(int id_teachers);

private:
    int m_id;
    QDateTime m_time_start;
    QDateTime m_time;
    QDateTime m_time_end;
    int m_id_subjects;
    int m_id_students;
    int m_id_teachers;
};

#endif // Kholle_H
