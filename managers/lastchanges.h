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

class LastChanges : public QDialog
{
    Q_OBJECT

public:
    explicit LastChanges(QSqlDatabase *db, QDate* monday, QWidget *parent = 0);
    ~LastChanges();
    bool free_kholleurs();
    bool free_timeslots();
    bool free_students();

public slots:
    bool change_timeslots(QListWidgetItem* item);
    bool update_timeslots();
    bool update_students();

private:
    Ui::LastChanges *ui;
    QSqlDatabase *m_db;
    QDate* m_monday;
    QQueue<Kholleur*> queue_displayedKholleurs; // Displayed teachers in the combobox "Kholleurs"
    QQueue<Timeslot*> queue_displayedTimeslots; // Displayed teachers in the list "Timeslots"
    QQueue<Student*> queue_displayedStudents; // Displayed teachers in the list "Students"
    QStringList days;
};

#endif // LASTCHANGES_H
