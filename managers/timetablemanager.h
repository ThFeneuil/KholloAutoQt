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
    ~TimetableManager();

    bool free_groups();
    bool update_list_groups();

    bool free_subjects();
    QMap<int, Subject*> update_list_subjects();

public slots:
    bool select_group();

protected:
    bool eventFilter(QObject* obj, QEvent *event);

private:
    Ui::TimetableManager *ui;
    QSqlDatabase *m_db;
    QQueue<Group*> queue_displayedGroups;
    QQueue<Subject*> queue_displayedSubjects;
    TimeTable* currentScene;
};

#endif // TIMETABLEMANAGER_H
