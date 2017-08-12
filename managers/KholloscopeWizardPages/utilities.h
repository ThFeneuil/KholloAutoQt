#ifndef UTILITIES_H
#define UTILITIES_H

#include <QtSql>
#include <QList>
#include <QMap>
#include <QDate>
#include "storedData/timeslot.h"
#include "storedData/student.h"
#include "database.h"

class Utilities
{
public:
    Utilities();

    static float proba(DataBase *dbase, Student* user, Timeslot* timeslot, QDate m_date);
    static QMap<int, float> *corrected_proba(DataBase *dbase, Student* user, QList<Timeslot *> list, QDate m_date);

    static bool compatible(QSqlDatabase *db, DataBase *dbase, int id_user, Timeslot* timeslot, int week, int id_kholle_avoid = 0);
    static void make_exchange(QSqlDatabase *db, Kholle *current, Timeslot *t_current, Kholle *k, Timeslot *t, int n1, int n2);
    static void quickSort(QList<Timeslot*> *list, int i, int j, QMap<int, float> *probas);

    static int listMax(DataBase *dbase, QList<Timeslot*>, QMap<int, float> *probas, Student*, QDate m_date);
    static void saveInSql(QSqlDatabase *db, QList<Kholle*>* kholloscope);

    static int sum_day(QSqlDatabase *db, int id_user, QDate date);
};

#endif // UTILITIES_H
