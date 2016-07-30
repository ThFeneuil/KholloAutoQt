#ifndef TEACHER_H
#define TEACHER_H

#include <QString>
#include <QList>
#include "storedData/subject.h"
#include "storedData/course.h"

class Subject;
class Course;

class Teacher
{
    public:
        Teacher();
        ~Teacher();

        //Getters
        int getId() const;
        QString getName() const;
        int getId_subjects() const;
        Subject* subject() const; // Interface
        QList<Course*>* courses() const; // Interface

        //Setters
        void setId(int id);
        void setName(QString name);
        void setId_subjects(int id_subjects);
        void setSubject(Subject* subj);

    private:
        int m_id;
        QString m_name;
        int m_id_subjects;

        //Interface
        Subject* m_subject;
        QList<Course*>* m_courses;
};

#endif // TEACHER_H
