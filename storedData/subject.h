#ifndef SUBJECT_H
#define SUBJECT_H

#include <QString>
#include <QList>
#include "storedData/teacher.h"
#include "storedData/kholleur.h"
#include "storedData/course.h"

class Teacher;
class Kholleur;
class Course;

class Subject
{
    public:
        Subject();
        ~Subject();

        //Getters
        int getId() const;
        QString getName() const;
        QString getShortName() const;
        QString getColor() const;
        QList<Teacher*>* teachers() const;
        QList<Kholleur*>* kholleurs() const;
        QList<Course*>* courses() const;

        //Setters
        void setId(int id);
        void setName(QString name);
        void setShortName(QString shortName);
        void setColor(QString color);

    private:
        int m_id;
        QString m_name;
        QString m_shortName;
        QString m_color;

        //Interface
        QList<Teacher*>* m_teachers;
        QList<Kholleur*>* m_kholleurs;
        QList<Course*>* m_courses;
};

#endif // SUBJECT_H
