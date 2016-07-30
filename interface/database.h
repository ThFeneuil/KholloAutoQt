#ifndef DATABASE_H
#define DATABASE_H

#include <QSql>
#include <QSqlQuery>
#include <QProgressBar>
#include <QMap>
#include <QVariant>
#include "storedData/student.h"
#include "storedData/group.h"
#include "storedData/subject.h"
#include "storedData/teacher.h"
#include "storedData/kholleur.h"
#include "storedData/course.h"
#include "storedData/timeslot.h"
#include "storedData/event.h"
#include "storedData/kholle.h"

class DataBase
{
public:
    DataBase(QSqlDatabase* db);
    ~DataBase();
public slots:
    bool load(QProgressBar* progressBar = NULL);
private:
    QSqlDatabase* m_db;
    QMap<int, Student*>* m_listStudents;
    QMap<int, Group*>* m_listGroups;
    QMap<int, Subject*>* m_listSubjects;
    QMap<int, Teacher*>* m_listTeachers;
    QMap<int, Kholleur*>* m_listKholleurs;
    QMap<int, Course*>* m_listCourses;
    QMap<int, Timeslot*>* m_listTimeslots;
    QMap<int, Event*>* m_listEvents;
    QMap<int, Kholle*>* m_listKholles;
};

#endif // DATABASE_H
