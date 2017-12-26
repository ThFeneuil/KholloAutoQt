#ifndef TIMETABLEMANAGER_H
#define TIMETABLEMANAGER_H

#include <QDialog>
#include <QtSql>
#include <QListWidgetItem>
#include <QQueue>
#include <QKeyEvent>
#include "managers/timetable.h"
#include "storedData/group.h"
#include "storedData/course.h"

namespace Ui {
class TimetableManager;
}

class TimetableManager : public QDialog
{
    Q_OBJECT
    enum DataImg { BeginDays, BeginHours, BetweenDays, BetweenHours };

public:
    explicit TimetableManager(QSqlDatabase *db, QWidget *parent = 0);
    // db : SQL database
    ~TimetableManager();

    bool free_groups(); // To free the groups of the memory
    bool update_list_groups(); // To get and display the groups

    bool free_subjects(); // To free the subjects of the memory
    QMap<int, Subject*> update_list_subjects(); // To get and display the subjects, return a map associating the id of subjects with the object

public slots:
    bool select_group(); // To select the group choose by the user

protected:
    bool eventFilter(QObject* obj, QEvent *event); // To catch events (press delete key)

private:
    Ui::TimetableManager *ui; // GUI
    QSqlDatabase *m_db; // SQL database
    QQueue<Group*> queue_displayedGroups; // Displayed groups in the list "Groups"
    QQueue<Subject*> queue_displayedSubjects; // Displayed subjects in the list "Subjects"
    TimeTable* currentScene; // Dynamic TimeTable
};

#endif // TIMETABLEMANAGER_H
