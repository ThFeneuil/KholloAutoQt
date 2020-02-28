#ifndef DATABASE_H
#define DATABASE_H

#include <QSqlQuery>
#include <QProgressBar>
#include <QMap>
#include <QVariant>
#include <QString>
#include <QMessageBox>
#include "storedData/student.h"
#include "storedData/group.h"
#include "storedData/subject.h"
#include "storedData/kholleur.h"
#include "storedData/course.h"
#include "storedData/timeslot.h"
#include "storedData/event.h"
#include "storedData/kholle.h"

class Student;
class Group;
class Subject;
class Kholleur;
class Course;
class Timeslot;
class Event;
class Kholle;

class DataBase
{
public:
    DataBase(QSqlDatabase* db);
    ~DataBase();
    QMap<int, Student*>* listStudents() const;
    QMap<int, Group*>* listGroups() const;
    QMap<int, Subject*>* listSubjects() const;
    QMap<int, Kholleur*>* listKholleurs() const;
    QMap<int, Course*>* listCourses() const;
    QMap<int, Timeslot*>* listTimeslots() const;
    QMap<int, Event*>* listEvents() const;
    QMap<int, Kholle*>* listKholles() const;

    void setConditionCourses(QString condition);
    void setConditionTimeslots(QString condition);


public slots:
    bool load(QProgressBar* progressBar = nullptr);
private:
    QSqlDatabase* m_db;
    QMap<int, Student*>* m_listStudents;
    QMap<int, Group*>* m_listGroups;
    QMap<int, Subject*>* m_listSubjects;
    QMap<int, Kholleur*>* m_listKholleurs;
    QMap<int, Course*>* m_listCourses;
    QMap<int, Timeslot*>* m_listTimeslots;
    QMap<int, Event*>* m_listEvents;
    QMap<int, Kholle*>* m_listKholles;

    QString m_conditionCourses;
    QString m_conditionTimeslots;
};

#endif // DATABASE_H
