#ifndef LASTCHANGES_H
#define LASTCHANGES_H

#include <QDialog>
#include <QtSql>
#include <QDate>
#include <QMessageBox>
#include <QListWidgetItem>
#include <QTableWidgetItem>
#include "interface/interfacedialog.h"
#include "managers/updatetimeslotdialog.h"
#include "storedData/kholleur.h"
#include "storedData/timeslot.h"
#include "storedData/student.h"
#include "notepad.h"

namespace Ui {
class LastChanges;
}

class LastChanges : public QDialog
{
    Q_OBJECT

enum Status {OK, Remove, Keep, NotKeep, ImpossibleToKeep};

struct TimeslotChg {
    Timeslot* start;
    Timeslot* end;
};

struct StudentKholleChg {
    Status status;
    QString statusMessage;
    Student* stdnt;
    TimeslotChg* tsChg;
    int numTs;
};

public:
    explicit LastChanges(QSqlDatabase *db, int id_week, QDate* monday, QWidget* parent = 0);
    ~LastChanges();
    bool free_timeslots();
    QString compatible(Student* stdnt, Timeslot *timeslot);

public slots:
    bool change_timeslots(QListWidgetItem* item);
    bool change_timeslotsList();
    bool update_timeslotsList(int idTs = -1);
    bool update_students(int idTs = -1);
    bool save_timeslotsChanges();
    bool open_interface();
    bool change_status_student(QTableWidgetItem* item);

    bool update_khollesManager();
    bool delete_kholles();

    bool reset_preferencesMerging();

private:
    Ui::LastChanges *ui;
    QSqlDatabase *m_db;
    QDate* m_monday;
    int m_id_week;
    QQueue<Kholleur*> queue_displayedKholleurs; // Displayed kholleurs in the combobox "Kholleurs"
    QQueue<TimeslotChg*> queue_displayedTimeslots; // Displayed timeslots in the list "Timeslots"
    QList<StudentKholleChg*> m_students; // Displayed students in the list "Students"
    QMap<Timeslot*, Timeslot*> m_mapTimeslots;
    QStringList days;
    QAction* m_shortcutNotepad;
};

#endif // LASTCHANGES_H
