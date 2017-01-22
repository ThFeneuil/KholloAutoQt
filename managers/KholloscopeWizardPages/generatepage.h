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
#include "utilities.h"

struct working_index {
    int current_student;
    int current_subject;
    int max;
};


enum ExchangeType {Collisions, Warnings, All};

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

    void constructPoss();

    QMap<int, QList<Timeslot*> > *updatePoss(int id_user, Timeslot *current);
    void resetPoss(int id_user, QMap<int, QList<Timeslot*> > *old);

    working_index *findMax();
    bool generate();

    void setStatus();
    bool exchange(int index, ExchangeType type, int score_limit);

    void display(int *errors, int *warnings);
    void displayCollision(int *collisions);
    void displayBlocking();
    void displayConclusion(int errors, int warnings, int collisions);

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
