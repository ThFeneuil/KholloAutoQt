#ifndef LASTCHANGES_H
#define LASTCHANGES_H

#include <QDialog>
#include <QtSql>
#include <QDate>
#include <QMessageBox>
#include <QListWidgetItem>
#include "managers/updatetimeslotdialog.h"
#include "storedData/kholleur.h"
#include "storedData/timeslot.h"
#include "storedData/student.h"

namespace Ui {
class LastChanges;
}

struct TimeslotChg {
    Timeslot* start;
    Timeslot* end;
};

class LastChanges : public QDialog
{
    Q_OBJECT

public:
    explicit LastChanges(QSqlDatabase *db, int id_week, QDate* monday, QWidget *parent = 0);
    ~LastChanges();
    bool free_kholleurs();
    bool free_timeslots();
    bool free_students();
    QString compatible(Student* stdnt, Timeslot *timeslot);

public slots:
    bool change_timeslots(QListWidgetItem* item);
    bool change_timeslotsList();
    bool update_timeslotsList();
    bool update_students();

    bool update_khollesManager();
    bool delete_kholles();

private:
    Ui::LastChanges *ui;
    QSqlDatabase *m_db;
    QDate* m_monday;
    int m_id_week;
    QQueue<Kholleur*> queue_displayedKholleurs; // Displayed teachers in the combobox "Kholleurs"
    QQueue<TimeslotChg*> queue_displayedTimeslots; // Displayed teachers in the list "Timeslots"
    QQueue<Student*> queue_displayedStudents; // Displayed teachers in the list "Students"
    QMap<Timeslot*, Timeslot*> m_mapTimeslots;
    QStringList days;
};

#endif // LASTCHANGES_H
