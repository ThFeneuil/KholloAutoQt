#ifndef GROUP_H
#define GROUP_H

#include <QString>
#include <QList>
#include "storedData/student.h"
#include "storedData/course.h"
#include "storedData/event.h"

class Student;
class Course;
class Event;

class Group
{
    public:
        Group();
        ~Group();

        //Getters
        int getId() const;
        QString getName() const;
        QList<Student*>* students() const; // Interface
        QList<Course*>* courses() const; // Interface
        QList<Event*>* events() const; // Interface

        //Setters
        void setId(int id);
        void setName(QString name);

    private:
        int m_id;
        QString m_name;

        // Interface
        QList<Student*>* m_students;
        QList<Course*>* m_courses;
        QList<Event*>* m_events;
};

#endif // GROUP_H
