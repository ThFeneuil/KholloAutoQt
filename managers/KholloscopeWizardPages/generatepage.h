#ifndef GENERATEPAGE_H
#define GENERATEPAGE_H

#include <QWizardPage>
#include <QtSql>
#include <QMessageBox>
#include <QQueue>
#include <QMap>
#include <QListWidget>
#include <QListWidgetItem>
#include "storedData/subject.h"
#include "storedData/student.h"
#include "storedData/timeslot.h"
#include "storedData/kholleur.h"
#include "storedData/kholle.h"
#include "managers/kholloscopewizard.h"

struct working_index {
    int current_student;
    int current_subject;
    int min;
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

    void getTimeslots();
    void freeTimeslots();

    void loadSubjects();

    void getKholleurs();
    void freeKholleurs();

    void calculateProba();

    bool compatible(int id_user, Timeslot* timeslot);
    void quickSort(QList<Timeslot*> list, int i, int j, int id_user);
    void constructPoss();

    QMap<int, QList<Timeslot*> > *updatePoss(int id_user, Timeslot *current);
    void resetPoss(int id_user, QMap<int, QList<Timeslot*> > *old);

    int my_count(QList<Timeslot*>);
    working_index *findMin();
    bool generate();

    void msg_display();
    void display();

private:
    Ui::GeneratePage *ui;
    QSqlDatabase *m_db;
    QList<Timeslot*> timeslots;
    QMap<int, Subject*> subjects;
    QMap<int, Kholleur*> kholleurs;
    int m_week;
    QDate m_date;

    QMap<int, QMap<int, float> > proba;
    QMap<int, QMap<int, QList<Timeslot*> > > poss;
    int current_student;
    int current_subject;
    int current_min;
    int profondeur;
    QList<Kholle*> kholloscope;
};

#endif // GENERATEPAGE_H