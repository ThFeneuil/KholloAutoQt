#ifndef GENERATEPAGE_H
#define GENERATEPAGE_H

#include <QWizardPage>
#include <QtSql>
#include <QMessageBox>
#include <QQueue>
#include <QMap>
#include <QListWidget>
#include <QListWidgetItem>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QStandardPaths>
#include <QPdfWriter>
#include <QPainter>
#include <QtConcurrentRun>
#include <math.h>
#include "storedData/subject.h"
#include "storedData/student.h"
#include "storedData/timeslot.h"
#include "storedData/kholleur.h"
#include "storedData/kholle.h"
#include "database.h"
#include "notepad.h"
#include "managers/kholloscopewizard.h"
#include "mainwindow.h"
#include "printpdf.h"

struct working_index {
    int current_student;
    int current_subject;
    int max;
};

namespace Ui {
class GeneratePage;
}

class GeneratePage : public QWizardPage
{
    Q_OBJECT

public:
    explicit GeneratePage(QSqlDatabase *db, QWidget *parent = 0);
    ~GeneratePage();
    void initializePage();
    void cleanupPage();
    void setPupilsOnTimeslots();

    QList<Subject *> *testAvailability();

    int weeksTo(Timeslot* ts1, Timeslot* ts2);
    float proba(Student* user, Timeslot* timeslot);
    QMap<int, float> *corrected_proba(Student* user, QList<Timeslot *> list);

    bool compatible(int id_user, Timeslot* timeslot);
    void quickSort(QList<Timeslot*> *list, int i, int j, QMap<int, float> *probas);
    void constructPoss();

    QMap<int, QList<Timeslot*> > *updatePoss(int id_user, Timeslot *current);
    void resetPoss(int id_user, QMap<int, QList<Timeslot*> > *old);

    int listMax(QList<Timeslot*>, Student*);
    working_index *findMax();
    bool generate();

    void saveInSql();
    void setStatus();
    bool exchange(int index, bool only_warnings, int score_limit);

    void msg_display();
    int nearestKholle(Student*, Timeslot*);
    void display();
    void displayCollision();
    void displayBlocking();

    void freeKholles();

public slots:
    void saveKholles();
    void finished();
    void abort();
    void show_notepad_collisions();
    void show_notepad_khollo();

private:
    Ui::GeneratePage *ui;
    QObject *m_window;
    QSqlDatabase *m_db;
    DataBase *m_dbase;
    QFile *log_file;
    QMap<int, Timeslot*> timeslots;
    QMap<int, Subject*> subjects;
    QMap<int, Kholleur*> kholleurs;
    int m_week;
    QDate m_date;

    QMap<int, QMap<int, QList<Timeslot*> > > poss;
    int profondeur;
    working_index last_index;
    QList<Kholle*> kholloscope;

    QMap<int, bool> m_downgraded;
    QString timestamp;
    QString khollo_message;
    QString collisions_message;

    bool m_abort;
    QFutureWatcher<bool> m_watcher;
    QMessageBox *m_box;
};

#endif // GENERATEPAGE_H
