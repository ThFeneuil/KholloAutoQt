#ifndef GENERATIONMETHOD_H
#define GENERATIONMETHOD_H

#include <QString>
#include <QFile>
#include <QStandardPaths>
#include <QtConcurrentRun>
#include <QSqlDatabase>

#include "managers/KholloscopeWizardPages/utilities.h"
#include "storedData/timeslot.h"
#include "database.h"

#define NAME_FILE_LOG "gen_log.txt"

class GenerationMethod
{
public:
    GenerationMethod(QSqlDatabase *db, QDate date, int week);
    virtual ~GenerationMethod();
    virtual bool start(QList<Subject*> *selected_subjects, QMap<int, QList<Student*> > *input) = 0;
    void commit();
    void rollback();
    void log(QString text, bool canBeDisplayed);

    float proba(Student* user, Timeslot* timeslot, QDate m_date);
    QDate date() const;
    int week() const;
    QMap<int, Student*>* listStudents() const;
    QMap<int, Timeslot*>* listTimeslots() const;
    bool compatible(int id_user, Timeslot* timeslot, int week, int id_kholle_avoid = 0, int *id_pb_kholle = NULL);
    QList<Kholle*>* kholloscope();
    Kholle* createKholle(Student* stud, Timeslot *ts);
    void setKhollesStatus();
    void saveInSql();
    QList<Subject*>* testAvailability(QList<Subject*> *selected_subjects, QMap<int, QList<Student*> > *input);

private:
    void setPupilsOnTimeslots();

private:
    QSqlDatabase *m_db;
    DataBase *m_dbase;
    QFile *m_log_file;
    QTextStream* m_out_log;
    int m_week;
    QDate m_date;

    QList<Kholle*> *m_kholloscope;
};

#endif // GENERATIONMETHOD_H
